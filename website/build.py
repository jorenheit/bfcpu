#!/usr/bin/env python3
"""Build the Synapse-191 report as a static, GitHub Pages-ready website."""

from __future__ import annotations

import argparse
import html
import json
import os
import posixpath
import re
import shutil
import subprocess
import sys
import textwrap
from pathlib import Path, PurePosixPath


HERE = Path(__file__).resolve().parent

ROUTES: dict[str, tuple[str, str, str]] = {
    "Abstract": ("/about/abstract/", "Front matter", "Abstract"),
    "Acknowledgement": ("/about/acknowledgements/", "Front matter", "Acknowledgement"),
    "Acknowledgements": ("/about/acknowledgements/", "Front matter", "Acknowledgements"),
    "Introduction": ("/synapse-191/introduction/", "The Brainfuck Computer", "01"),
    "Synapse-191 Architecture": ("/synapse-191/architecture/", "The Brainfuck Computer", "02"),
    "Opcodes and Control Sequences": ("/synapse-191/opcodes-control-sequences/", "The Brainfuck Computer", "03"),
    "Hardware Implementation": ("/synapse-191/hardware-implementation/", "The Brainfuck Computer", "04"),
    "Supporting Tools": ("/synapse-191/supporting-tools/", "The Brainfuck Computer", "05"),
    "Runtime Results": ("/synapse-191/runtime-results/", "The Brainfuck Computer", "06"),
    "Introduction to Acus": ("/acus/introduction/", "Brainfuck Compilation", "01"),
    "Acus Program Model": ("/acus/program-model/", "Brainfuck Compilation", "02"),
    "Tape Abstractions": ("/acus/tape-abstractions/", "Brainfuck Compilation", "03"),
    "Memory Management": ("/acus/memory-management/", "Brainfuck Compilation", "04"),
    "Code Generation": ("/acus/code-generation/", "Brainfuck Compilation", "05"),
    "Caching": ("/acus/caching/", "Brainfuck Compilation", "06"),
    "Other Optimizations": ("/acus/other-optimizations/", "Brainfuck Compilation", "07"),
    "Final Thoughts and Conclusion": ("/reflection/final-thoughts/", "Reflection", "01"),
    "Microcode Table": ("/appendix/microcode-table/", "Appendix", "A"),
    "Mugen Specification": ("/appendix/mugen-specification/", "Appendix", "B"),
    "Bill of Materials": ("/appendix/bill-of-materials/", "Appendix", "C"),
    "BF Test Suite": ("/appendix/bf-test-suite/", "Appendix", "D"),
    "IO Module ISR": ("/appendix/io-module-isr/", "Appendix", "E"),
    "Constant Factory Lookup Table": ("/appendix/constant-factory-table/", "Appendix", "F"),
    "Schematics": ("/appendix/schematics/", "Appendix", "G"),
    "Flashback": ("/appendix/flashback/", "Appendix", "H"),
    "References": ("/references/", "Back matter", "References"),
}

PART_ORDER = [
    "The Brainfuck Computer",
    "Brainfuck Compilation",
    "Reflection",
    "Appendix",
    "Front matter",
    "Back matter",
]

LANDINGS = {
    "/synapse-191/": {
        "key": "synapse",
        "eyebrow": "Part I · The Brainfuck Computer",
        "title": "A native Brainfuck computer, built in TTL logic.",
        "lead": "Synapse-191 turns Brainfuck’s eight commands into a physical instruction set: a microcoded breadboard CPU with separate program and data memory, a two-phase clock, and dedicated I/O.",
        "part": "The Brainfuck Computer",
        "accent": "+  −  <  >  [  ]  .  ,",
    },
    "/acus/": {
        "key": "acus",
        "eyebrow": "Part II · Brainfuck Compilation",
        "title": "Acus: A Typed C++ Compiler Backend for Generating Brainfuck",
        "lead": "Acus provides the abstractions needed to translate higher-level programs into Brainfuck: types, variables, arrays, structs, pointers, functions, stack frames, control flow, and optimization.",
        "part": "Brainfuck Compilation",
        "accent": "Cell → MacroCell → Slot → Frame → Tape",
    },
    "/appendix/": {
        "key": "appendix",
        "eyebrow": "Technical appendices",
        "title": "Tables, source listings, schematics, and build records.",
        "lead": "The appendices collect the complete microcode table, Mugen specification, bill of materials, test suite, I/O interrupt code, constant lookup table, schematics, and a photographic flashback.",
        "part": "Appendix",
        "accent": "A  B  C  D  E  F  G  H",
    },
    "/reflection/": {
        "key": "reflection",
        "eyebrow": "Part III · Reflection",
        "title": "What worked, what changed, and what comes next.",
        "lead": "A retrospective on the design choices, practical lessons, possible improvements, and the surprising amount of rigor required to build something deliberately impractical.",
        "part": "Reflection",
        "accent": "BUILD → TEST → REVISE",
    },
    "/about/": {
        "key": "about",
        "eyebrow": "Front matter",
        "title": "About this report and the people behind it.",
        "lead": "Read the abstract for a compact project overview, then the acknowledgement of Artur Topal’s vital role in the project’s early design and construction.",
        "part": "Front matter",
        "accent": "SYN-191 · WEB EDITION",
    },
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, help="directory containing main.tex")
    parser.add_argument("--output", type=Path, default=HERE / "_site")
    parser.add_argument("--site-url", default=os.environ.get("SITE_URL", ""))
    return parser.parse_args()


def find_source(configured: Path | None) -> Path:
    candidates = [
        configured,
        HERE.parent / "doc",
        Path.cwd() / "doc",
        HERE / "content" / "tex",
    ]
    for candidate in candidates:
        if candidate and (candidate / "main.tex").is_file():
            return candidate.resolve()
    raise SystemExit("No main.tex found. Pass --source doc or keep the bundled content/tex snapshot.")


def safe_output_directory(path: Path) -> Path:
    path = path.resolve()
    if path == Path("/") or path == Path.home() or len(path.parts) < 3:
        raise SystemExit(f"Refusing unsafe output directory: {path}")
    if path.exists():
        marker = path / ".synapse-report-output"
        if path.name != "_site" and not marker.exists():
            raise SystemExit(f"Refusing to replace unmarked directory: {path}")
        shutil.rmtree(path)
    path.mkdir(parents=True)
    (path / ".synapse-report-output").write_text("generated; safe to replace\n", encoding="utf-8")
    return path


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def strip_html(value: str) -> str:
    value = re.sub(r"<script\b[^>]*>.*?</script>", " ", value, flags=re.S | re.I)
    value = re.sub(r"<style\b[^>]*>.*?</style>", " ", value, flags=re.S | re.I)
    value = re.sub(r"<annotation\b[^>]*>.*?</annotation>", " ", value, flags=re.S | re.I)
    value = re.sub(r"<[^>]+>", " ", value)
    return re.sub(r"\s+", " ", html.unescape(value)).strip()


def slugify(value: str) -> str:
    value = strip_html(value).lower()
    return re.sub(r"[^a-z0-9]+", "-", value).strip("-") or "chapter"


def route_directory(route: str) -> str:
    return route.strip("/") or "."


def relative_url(current_route: str, target_route: str, fragment: str = "") -> str:
    current = route_directory(current_route)
    target = route_directory(target_route)
    relative = posixpath.relpath(target, current)
    if relative == ".":
        relative = "./"
    else:
        relative += "/"
    return relative + (f"#{fragment}" if fragment else "")


def asset_url(route: str, asset: str) -> str:
    return posixpath.relpath(asset, route_directory(route))


def route_output_path(output: Path, route: str) -> Path:
    return output / route.strip("/") / "index.html" if route != "/" else output / "index.html"


def parse_references(source: str) -> dict[str, int]:
    parts = re.split(r"\\bibitem\s*\{([^}]+)\}", source)
    keys = [parts[index].strip() for index in range(1, len(parts), 2)]
    return {key: index + 1 for index, key in enumerate(keys)}


def references_as_latex(source: str) -> str:
    source = re.sub(
        r"\\begin\{thebibliography\}\{[^}]+\}",
        r"\\chapter{References}\\label{resources}\n\\begin{enumerate}",
        source,
        count=1,
    )
    source = source.replace(r"\end{thebibliography}", r"\end{enumerate}")
    return re.sub(
        r"\\bibitem\s*\{([^}]+)\}",
        lambda match: rf"\item \hypertarget{{ref-{match.group(1)}}}{{}}",
        source,
    )


def expand_includes(text: str, source_dir: Path) -> str:
    pattern = re.compile(r"\\include\{([^}]+)\}")

    def replace(match: re.Match[str]) -> str:
        name = match.group(1)
        path = source_dir / f"{name}.tex"
        if not path.is_file():
            return rf"\chapter{{Missing source: {name}}}"
        source = read_text(path)
        return references_as_latex(source) if name == "references" else expand_includes(source, source_dir)

    return pattern.sub(replace, text)


def select_listing_lines(source: str, options: str) -> str:
    lines = source.splitlines()
    first = re.search(r"firstline\s*=\s*(\d+)", options)
    last = re.search(r"lastline\s*=\s*(\d+)", options)
    if first or last:
        start = max(0, int(first.group(1)) - 1) if first else 0
        end = int(last.group(1)) if last else len(lines)
        lines = lines[start:end]
    marker = re.search(r"linerange\s*=\s*\{([^{}]+)\}", options)
    if marker and "-" in marker.group(1):
        start_marker, end_marker = marker.group(1).replace(r"\_", "_").split("-", 1)
        start = next((i + 1 for i, line in enumerate(lines) if start_marker in line), 0)
        end = next((i for i, line in enumerate(lines[start:], start) if end_marker in line), len(lines))
        lines = lines[start:end]
    return "\n".join(lines)


def expand_listings(text: str, source_dir: Path) -> tuple[str, list[str]]:
    pattern = re.compile(r"\\lstinputlisting(?:\[([^]]*)\])?\{([^}]+)\}")
    missing: list[str] = []

    def replace(match: re.Match[str]) -> str:
        options = match.group(1) or ""
        raw = match.group(2).replace(r"\_", "_")
        path = (source_dir / raw).resolve()
        language = re.search(r"(?:language|style)\s*=\s*([^,]+)", options, flags=re.I)
        language_name = language.group(1).strip() if language else "text"
        if path.is_file():
            listing = select_listing_lines(read_text(path), options)
        else:
            missing.append(raw)
            comment = "#" if path.suffix in {".bf", ".mu"} else "//"
            listing = f"{comment} Source listing: {raw}\n{comment} Populated when built inside the full Synapse repository."
        return f"\\begin{{lstlisting}}[language={language_name}]\n{listing}\n\\end{{lstlisting}}"

    return pattern.sub(replace, text), sorted(set(missing))


LISTING_STYLES = {
    "cpp": "C++",
    "pseudocode": "Python",
    "brainfuck": "brainfuck",
    "mugenstyle": "mugen",
}


def normalize_listings(source: str) -> str:
    """Translate LaTeX listing styles to web languages and remove layout indent."""
    pattern = re.compile(
        r"\\begin\{lstlisting\}(?:\[([^]]*)\])?\s*\n(.*?)\\end\{lstlisting\}",
        re.S,
    )

    def replace(match: re.Match[str]) -> str:
        options = match.group(1) or ""
        body = textwrap.dedent(match.group(2)).rstrip()
        language = re.search(r"language\s*=\s*([^,]+)", options, flags=re.I)
        style = re.search(r"style\s*=\s*([^,]+)", options, flags=re.I)
        if language:
            web_language = language.group(1).strip()
        elif style:
            name = style.group(1).strip()
            web_language = LISTING_STYLES.get(name.lower(), name)
        else:
            web_language = "text"
        return f"\\begin{{lstlisting}}[language={web_language}]\n{body}\n\\end{{lstlisting}}"

    return pattern.sub(replace, source)


def command_argument(source: str, command: str, start: int = 0) -> tuple[str, int, int] | None:
    """Return a balanced braced argument and its source range."""
    match = re.search(rf"\\{re.escape(command)}\s*\{{", source[start:])
    if not match:
        return None
    open_brace = start + match.end() - 1
    depth = 0
    escaped = False
    for index in range(open_brace, len(source)):
        character = source[index]
        if escaped:
            escaped = False
            continue
        if character == "\\":
            escaped = True
            continue
        if character == "{":
            depth += 1
        elif character == "}":
            depth -= 1
            if depth == 0:
                return source[open_brace + 1:index], start + match.start(), index + 1
    return None


def latex_inline_text(source: str) -> str:
    value = source
    for _ in range(4):
        value = re.sub(r"\\(?:texttt|textbf|emph|textrm|textit)\{([^{}]*)\}", r"\1", value)
    value = re.sub(r"\\(?:ref|autoref|pageref)\{[^}]+\}", "the linked section", value)
    value = re.sub(r"\\cite\{[^}]+\}", "", value)
    value = value.replace(r"\%", "%").replace(r"\_", "_").replace(r"\&", "&")
    value = re.sub(r"\\[A-Za-z]+", "", value)
    value = value.replace("$", "").replace("{", "").replace("}", "")
    return re.sub(r"\s+", " ", value).strip()


def collect_source_label_titles(source: str) -> dict[str, str]:
    titles: dict[str, str] = {}
    cursor = 0
    while True:
        caption = command_argument(source, "caption", cursor)
        if not caption:
            break
        value, _, end = caption
        following = source[end:end + 500]
        label = re.search(r"\\label\{([^}]+)\}", following)
        if label:
            titles.setdefault(label.group(1), latex_inline_text(value) or "related item")
        cursor = end
    return titles


def normalize_custom_environments(source: str) -> str:
    """Lower LaTeX environments that Pandoc's reader otherwise discards."""
    source = re.sub(r"\\begin\{wrapfigure\}\{[^}]*\}\{[^}]*\}", r"\\begin{figure}", source)
    source = source.replace(r"\end{wrapfigure}", r"\end{figure}")
    pattern = re.compile(r"\\begin\{lstfloat\}(?:\[[^]]*\])?(.*?)\\end\{lstfloat\}", re.S)

    def listing_float(match: re.Match[str]) -> str:
        body = match.group(1)
        caption = command_argument(body, "caption")
        label_match = re.search(r"\\label\{([^}]+)\}", body)
        caption_text = caption[0] if caption else "Source listing"
        if caption:
            body = body[:caption[1]] + body[caption[2]:]
        body = re.sub(r"\\label\{[^}]+\}", "", body)
        anchor = f"\\hypertarget{{{label_match.group(1)}}}{{}}\n" if label_match else ""
        return anchor + body.strip() + f"\n\n\\emph{{Listing: {caption_text}}}\n"

    return pattern.sub(listing_float, source)


def preprocess_latex(source: str, source_dir: Path, citations: dict[str, int]) -> tuple[str, list[str]]:
    source = normalize_custom_environments(source)
    source = re.sub(r"\\part\{[^}]+\}", "", source)
    source = re.sub(r"\\(?:frontmatter|mainmatter|backmatter|appendix)\b", "", source)
    source = re.sub(r"\\begin\{labeledenum\}\{[^}]+\}(?:\[[^]]+\])?", r"\\begin{enumerate}", source)
    source = source.replace(r"\end{labeledenum}", r"\end{enumerate}")
    source = source.replace(r"\center", r"\centering")
    source = re.sub(r"\\(?:newpage|clearpage|vfill|mbox\{\})", "", source)
    source = re.sub(
        r"\\(?:ref|autoref|pageref)\{([^}]+)\}",
        lambda match: rf"\href{{XREF:{match.group(1)}}}{{reference}}",
        source,
    )
    source = re.sub(
        r"\\label\{(lst:[^}]+)\}",
        lambda match: rf"\hypertarget{{{match.group(1)}}}{{}}",
        source,
    )

    def citation(match: re.Match[str]) -> str:
        links = []
        for key in [value.strip() for value in match.group(1).split(",")]:
            links.append(rf"\href{{CITE:{key}}}{{[{citations.get(key, '?')}]}}")
        return ", ".join(links)

    source = re.sub(r"\\cite\{([^}]+)\}", citation, source)

    def schematic(match: re.Match[str]) -> str:
        target = match.group(1).replace(r"\_", "_")
        label = Path(target).stem.replace("_", " ").title()
        return rf"\paragraph{{{label}}} \href{{SCHEMATIC:{target}}}{{Open the original PDF schematic.}}"

    source = re.sub(r"\\includepdf(?:\[[^]]*\])?\{([^}]+)\}", schematic, source)
    source, missing = expand_listings(source, source_dir)
    return normalize_listings(source), missing


def run_pandoc(source: str) -> str:
    pandoc = shutil.which("pandoc")
    if not pandoc:
        raise SystemExit("pandoc is required. Install it with your package manager and rerun the build.")
    result = subprocess.run(
        [pandoc, "--from=latex+raw_tex", "--to=html5", "--mathml", "--wrap=none"],
        input=source,
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode:
        raise SystemExit(result.stderr.strip() or "pandoc conversion failed")
    return result.stdout


def split_pages(converted: str) -> list[dict[str, object]]:
    pages: list[dict[str, object]] = []
    for chunk in re.split(r"(?=<h1\b)", converted):
        heading = re.match(r"<h1\b([^>]*)>(.*?)</h1>", chunk, flags=re.S)
        if not heading:
            continue
        title = strip_html(heading.group(2))
        route, part, number = ROUTES.get(title, (f"/chapters/{slugify(title)}/", "Report", str(len(pages) + 1)))
        chapter_id = (re.search(r'\bid="([^"]+)"', heading.group(1)) or [None, slugify(title)])[1]
        body = chunk[heading.end():].strip()
        sections = [
            {"level": 2 if level == "h2" else 3, "id": identifier, "title": strip_html(label)}
            for level, identifier, label in re.findall(
                r"<(h2|h3)\b[^>]*\bid=\"([^\"]+)\"[^>]*>(.*?)</\1>", body, flags=re.S
            )
        ]
        plain = strip_html(body)
        paragraph_texts = [strip_html(value) for value in re.findall(r"<p>(.*?)</p>", body, flags=re.S)]
        description = next((value for value in paragraph_texts if len(value) > 45), plain)
        if len(description) > 230:
            description = description[:227].rsplit(" ", 1)[0] + "…"
        words = re.findall(r"[A-Za-z0-9][A-Za-z0-9'’-]*", plain)
        pages.append({
            "title": title,
            "chapterId": chapter_id,
            "route": route,
            "part": part,
            "number": number,
            "description": description,
            "minutes": max(1, round(len(words) / 220)),
            "sections": sections,
            "html": body,
            "searchText": plain,
        })
    return pages


def distribute_footnotes(pages: list[dict[str, object]]) -> None:
    notes: dict[str, str] = {}
    for page in pages:
        body = str(page["html"])
        aside = re.search(r'<aside\b[^>]*\bid="footnotes"[^>]*>.*?</aside>', body, flags=re.S | re.I)
        if not aside:
            continue
        notes.update(dict(re.findall(r'<li\b[^>]*\bid="fn(\d+)"[^>]*>(.*?)</li>', aside.group(0), flags=re.S | re.I)))
        page["html"] = body[:aside.start()] + body[aside.end():]
    for page in pages:
        body = str(page["html"])
        used = list(dict.fromkeys(re.findall(r'id="fnref(\d+)"', body)))
        items = []
        for number in used:
            if number not in notes:
                continue
            item = re.sub(rf'href="[^"]*#fnref{number}"', f'href="#fnref{number}"', notes[number])
            items.append(f'<li id="fn{number}">{item}</li>')
        if items:
            page["html"] = body + '\n<aside id="footnotes" class="footnotes" role="doc-endnotes"><hr><ol>' + "".join(items) + "</ol></aside>"


def collect_labels(pages: list[dict[str, object]]) -> tuple[dict[str, str], dict[str, str]]:
    owners: dict[str, str] = {}
    labels: dict[str, str] = {}
    for page in pages:
        route = str(page["route"])
        chapter_id = str(page["chapterId"])
        owners.setdefault(chapter_id, route)
        labels.setdefault(chapter_id, str(page["title"]))
        body = str(page["html"])
        for identifier in re.findall(r'\bid="([^"]+)"', body):
            owners.setdefault(identifier, route)
            if identifier.startswith("lst:"):
                default = "code example"
            elif identifier.startswith("fig:"):
                default = "related figure"
            elif identifier.startswith("tab:"):
                default = "related table"
            else:
                default = identifier.split(":")[-1].replace("-", " ")
            labels.setdefault(identifier, default)
        for identifier, label in re.findall(r'<h[2-5]\b[^>]*\bid="([^"]+)"[^>]*>(.*?)</h[2-5]>', body, flags=re.S | re.I):
            labels[identifier] = strip_html(label)
        for identifier, figure in re.findall(r'<figure\b[^>]*\bid="([^"]+)"[^>]*>(.*?)</figure>', body, flags=re.S | re.I):
            caption = re.search(r"<figcaption[^>]*>(.*?)</figcaption>", figure, flags=re.S | re.I)
            if caption:
                labels[identifier] = strip_html(caption.group(1))
        for identifier, table in re.findall(r'<div\b[^>]*\bid="([^"]+)"[^>]*>(.*?<table.*?</table>.*?)</div>', body, flags=re.S | re.I):
            caption = re.search(r"<caption[^>]*>(.*?)</caption>", table, flags=re.S | re.I)
            if caption:
                labels[identifier] = strip_html(caption.group(1))
    return owners, labels


def collect_source_label_owners(source: str) -> dict[str, str]:
    """Recover labels Pandoc cannot attach to listings, equations, or long tables."""
    matches = list(re.finditer(r"\\chapter\*?\{([^}]+)\}", source))
    owners: dict[str, str] = {}
    for index, match in enumerate(matches):
        title = re.sub(r"\\texttt\{([^}]+)\}", r"\1", match.group(1))
        title = re.sub(r"\\[A-Za-z]+", "", title).strip()
        route = ROUTES.get(title, (f"/chapters/{slugify(title)}/", "Report", ""))[0]
        end = matches[index + 1].start() if index + 1 < len(matches) else len(source)
        chunk = source[match.start():end]
        identifiers = re.findall(r"\\label\{([^}]+)\}", chunk)
        identifiers += re.findall(r"\\hypertarget\{([^}]+)\}\{\}", chunk)
        for identifier in identifiers:
            owners.setdefault(identifier, route)
    return owners


def add_fallback_anchors(pages: list[dict[str, object]], owners: dict[str, str]) -> None:
    for page in pages:
        body = str(page["html"])
        existing = {html.unescape(value) for value in re.findall(r'\bid="([^"]+)"', body)}
        missing = sorted(
            identifier for identifier, route in owners.items()
            if route == page["route"] and identifier not in existing and identifier != page["chapterId"]
        )
        if missing:
            anchors = "".join(
                f'<span class="legacy-anchor" id="{html.escape(identifier)}" aria-hidden="true"></span>'
                for identifier in missing
            )
            page["html"] = anchors + body


def resolve_source_asset(source_dir: Path, original: str) -> Path | None:
    relative = Path(html.unescape(original).replace(r"\_", "_"))
    candidate = source_dir / relative
    if candidate.is_file():
        return candidate
    if not candidate.suffix and candidate.parent.is_dir():
        matches = [
            path for path in sorted(candidate.parent.glob(candidate.name + ".*"))
            if path.suffix.lower() in {".png", ".jpg", ".jpeg", ".webp", ".svg", ".gif"}
        ]
        return matches[0] if matches else None
    return None


def autolink_reference_urls(page: dict[str, object]) -> None:
    if page["route"] != "/references/":
        return

    def replace(match: re.Match[str]) -> str:
        url = match.group(0)
        trailing = ""
        while url and url[-1] in ",.":
            trailing = url[-1] + trailing
            url = url[:-1]
        return f'<a href="{url}">{url}</a>{trailing}'

    page["html"] = re.sub(r"https?://[^<\s]+", replace, str(page["html"]))


def rewrite_page_html(
    page: dict[str, object],
    pages: list[dict[str, object]],
    owners: dict[str, str],
    labels: dict[str, str],
    source_dir: Path,
) -> None:
    current = str(page["route"])
    body = str(page["html"])

    def xref(match: re.Match[str]) -> str:
        identifier = match.group(1)
        target = owners.get(identifier, current)
        label = labels.get(identifier, "related section")
        return f'<a href="{relative_url(current, target, identifier)}" class="xref">{html.escape(label)}</a>'

    body = re.sub(r'<a href="XREF:([^"]+)"[^>]*>.*?</a>', xref, body, flags=re.S | re.I)

    def citation(match: re.Match[str]) -> str:
        key, label = match.group(1), match.group(2)
        return f'<a href="{relative_url(current, "/references/", f"ref-{key}")}" class="citation">{label}</a>'

    body = re.sub(r'<a href="CITE:([^"]+)"[^>]*>(.*?)</a>', citation, body, flags=re.S | re.I)

    def local_ref(match: re.Match[str]) -> str:
        identifier = match.group(1)
        owner = owners.get(identifier)
        return f'href="{relative_url(current, owner, identifier)}"' if owner and owner != current else match.group(0)

    body = re.sub(r'href="#([^"]+)"', local_ref, body)

    image_pattern = re.compile(r'<img\b[^>]*\bsrc="([^"]+)"[^>]*>', re.I)

    def image_replace(match: re.Match[str]) -> str:
        tag, original = match.group(0), match.group(1)
        resolved = resolve_source_asset(source_dir, original)
        if resolved:
            relative = resolved.relative_to(source_dir).as_posix()
            return tag.replace(f'src="{original}"', f'src="{asset_url(current, f"assets/report/{relative}")}"')
        label = Path(html.unescape(original)).name.replace("_", " ")
        return (
            f'<span class="missing-figure" role="img" data-missing-asset="{html.escape(original)}">'
            f'<span class="missing-figure-mark">FIG.</span><span>{html.escape(label)}</span></span>'
        )

    body = image_pattern.sub(image_replace, body)

    def schematic(match: re.Match[str]) -> str:
        target = match.group(1)
        path = source_dir / target
        if path.is_file():
            return f'href="{asset_url(current, f"assets/report/{target}")}"'
        return f'href="#" class="missing-schematic" data-missing-asset="{html.escape(target)}"'

    body = re.sub(r'href="SCHEMATIC:([^"]+)"', schematic, body)
    page["html"] = body


def refresh_page_metadata(page: dict[str, object]) -> None:
    body = str(page["html"])
    plain = strip_html(body)
    paragraphs = [strip_html(value) for value in re.findall(r"<p(?:\s[^>]*)?>(.*?)</p>", body, flags=re.S)]
    captions = [strip_html(value) for value in re.findall(r"<figcaption(?:\s[^>]*)?>(.*?)</figcaption>", body, flags=re.S)]
    description = next((value for value in [*paragraphs, *captions] if len(value) > 45), plain)
    if len(description) > 230:
        description = description[:227].rsplit(" ", 1)[0] + "…"
    words = re.findall(r"[A-Za-z0-9][A-Za-z0-9'’-]*", plain)
    page["description"] = description
    page["minutes"] = max(1, round(len(words) / 220))
    page["searchText"] = plain


def deduplicate_page_ids(page: dict[str, object]) -> None:
    seen: dict[str, int] = {}

    def replace(match: re.Match[str]) -> str:
        identifier = html.unescape(match.group(1))
        seen[identifier] = seen.get(identifier, 0) + 1
        if seen[identifier] == 1:
            return match.group(0)
        suffix = seen[identifier]
        return f'id="{html.escape(identifier)}-{suffix}"'

    page["html"] = re.sub(r'\bid="([^"]+)"', replace, str(page["html"]))


def copy_assets(source_dir: Path, output: Path) -> None:
    assets = output / "assets"
    assets.mkdir(parents=True, exist_ok=True)

    shutil.copy2(HERE / "assets" / "site.css", assets / "site.css")
    shutil.copy2(HERE / "assets" / "site.js", assets / "site.js")
    shutil.copy2(HERE / "assets" / "favicon.svg", assets / "favicon.svg")

    report_assets = assets / "report"
    for name in ("img", "schematics"):
        source = source_dir / name
        if source.is_dir():
            shutil.copytree(source, report_assets / name, dirs_exist_ok=True)

    # Copy the downloadable PDF
    pdf = source_dir / "synapse.pdf"
    if pdf.is_file():
        shutil.copy2(pdf, output / "synapse-191.pdf")
        

def header_html(route: str) -> str:
    nav = [
        ("Hardware", "/synapse-191/", route.startswith("/synapse-191")),
        ("Acus", "/acus/", route.startswith("/acus")),
        ("Appendix", "/appendix/", route.startswith("/appendix")),
    ]
    links = "".join(
        f'<a href="{relative_url(route, target)}"{(" aria-current=\"page\"" if active else "")}>{label}</a>'
        for label, target, active in nav
    )
    return f"""
<header class="site-header">
  <a class="site-brand" href="{relative_url(route, '/')}">
    <svg aria-hidden="true" viewBox="0 0 32 32"><rect x="7" y="7" width="18" height="18"/><circle cx="16" cy="16" r="3"/><path d="M5 2v5M5 25v5M11 2v5M11 25v5M17 2v5M17 25v5M23 2v5M23 25v5M2 5h5M25 5h5M2 11h5M25 11h5M2 17h5M25 17h5M2 23h5M25 23h5"/></svg>
    <span>Synapse-191</span>
  </a>
  <nav class="desktop-nav" aria-label="Primary navigation">{links}</nav>
  <div class="header-actions">
    <button class="search-trigger" type="button" data-search-open aria-label="Search the report"><span class="search-icon">⌕</span><span>Search</span><kbd>⌘ K</kbd></button>
    <button class="menu-trigger" type="button" data-menu-open aria-label="Toggle navigation" aria-expanded="false"><span></span><span></span></button>
  </div>
  <nav class="mobile-nav" aria-label="Mobile navigation">{links}<button type="button" data-search-open>Search the report</button></nav>
</header>"""


def footer_html(route: str) -> str:
    return f"""
<footer class="site-footer">
  <div><span class="footer-mark">[&nbsp;]</span><p><strong>Synapse-191</strong><br>Building a Brainfuck Computing Environment</p></div>
  <nav aria-label="Footer navigation">
    <a href="{relative_url(route, '/about/abstract/')}">Abstract</a>
    <a href="{relative_url(route, '/references/')}">References</a>
    <a href="https://github.com/jorenheit/bfcpu">Synapse source</a>
    <a href="https://github.com/jorenheit/acus">Acus source</a>
  </nav>
  <p class="footer-note">Web edition by Joren Heit · 2025</p>
</footer>"""


def document_html(route: str, title: str, description: str, main: str, site_url: str = "") -> str:
    canonical = ""
    if site_url:
        canonical_url = site_url.rstrip("/") + ("/" if route == "/" else route)
        canonical = f'<link rel="canonical" href="{html.escape(canonical_url)}">'
    root_prefix = posixpath.relpath(".", route_directory(route))
    if root_prefix == ".":
        root_prefix = ""
    else:
        root_prefix += "/"
    return f"""<!doctype html>
<html lang="en" data-root-prefix="{root_prefix}">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{html.escape(title)} · Synapse-191</title>
  <meta name="description" content="{html.escape(description)}">
  <meta name="author" content="Joren Heit">
  <meta name="theme-color" content="#f4f0e6">
  {canonical}
  <link rel="icon" href="{asset_url(route, 'assets/favicon.svg')}" type="image/svg+xml">
  <link rel="stylesheet" href="{asset_url(route, 'assets/site.css')}">
  <script defer src="{asset_url(route, 'assets/site.js')}"></script>
</head>
<body>
  <a class="skip-link" href="#main-content">Skip to content</a>
  {header_html(route)}
  <div class="reading-progress" aria-hidden="true"><span></span></div>
  <div id="main-content">{main}</div>
  {footer_html(route)}
  <div class="search-backdrop" data-search-dialog hidden>
    <section class="search-dialog" role="dialog" aria-modal="true" aria-label="Search the report">
      <div class="search-field"><span>⌕</span><input type="search" data-search-input placeholder="Search chapters, concepts, and code…" aria-label="Search query"><button type="button" data-search-close>Esc</button></div>
      <div class="search-results" data-search-results aria-live="polite"></div>
    </section>
  </div>
</body>
</html>"""


def hero_diagram() -> str:
    cells = ["…", "&gt;", "+", "+", "+", ".", "−", "&lt;", "]", "&lt;", "…"]
    cell_html = "".join(f'<span class="{"active" if index == 8 else ""}">{cell}</span>' for index, cell in enumerate(cells))
    return f"""
<div class="hero-diagram" aria-label="Brainfuck tape merging into circuit traces">
  <svg viewBox="0 0 720 650" role="img" aria-label="Brainfuck tape and circuit abstraction">
    <g class="diagram-grid">{''.join(f'<path d="M{i*60} 0v650"/><path d="M0 {i*60}h720"/>' for i in range(12))}</g>
    <g class="diagram-orbits"><circle cx="370" cy="325" r="170"/><circle cx="370" cy="325" r="215"/><path d="M370 95v92M370 463v92M140 325h92M508 325h92"/></g>
    <g class="diagram-circuit copper"><path d="M142 152h104l46 46v54M580 132h-88l-44 44v80M174 508h98l42-42v-55M570 520h-80l-60-60v-49"/><circle cx="142" cy="152" r="5"/><circle cx="580" cy="132" r="5"/></g>
    <g class="diagram-circuit blue"><path d="M218 282v-58h222v69l52 52M492 345v88H274v-70"/><circle cx="218" cy="282" r="5"/><circle cx="274" cy="363" r="5"/></g>
    <g class="diagram-registers"><text x="540" y="205">PC  0x0042</text><text x="540" y="226">DP  0x001F</text><text x="540" y="247">IR  0x2B</text><text x="120" y="455">ACC  0x00</text><text x="120" y="476">FLG  0x00</text></g>
  </svg>
  <div class="tape-row" aria-hidden="true">{cell_html}</div>
</div>"""


def arrow() -> str:
    return '<span aria-hidden="true">→</span>'


def render_home(pages: list[dict[str, object]]) -> str:
    groups = [(part, [p for p in pages if p["part"] == part]) for part in PART_ORDER[:4]]
    reading_map = "".join(
        f'<div><span>{index:02d}</span><h3>{html.escape(part)}</h3><p>{len(group)} {"chapter" if len(group) == 1 else "chapters"}</p><a href="{relative_url("/", str(group[0]["route"]))}">Open section {arrow()}</a></div>'
        for index, (part, group) in enumerate(groups, 1) if group
    )
    return f"""
<main>
  <section class="home-hero">
    <div class="home-hero-copy">
      <p class="eyebrow">Technical report · Web edition</p>
      <h1>Synapse-191</h1>
      <h2>Building a Brainfuck<br>Computing Environment</h2>
      <p class="hero-lead">From a native Brainfuck computer to a typed compiler backend.</p>

      <div class="hero-actions">
        <a class="button button-primary"
           href="{relative_url('/', '/synapse-191/introduction/')}">
          Start reading {arrow()}
        </a>
        <a class="button button-secondary"
           href="{relative_url('/', '/acus/')}">
          Explore Acus {arrow()}
        </a>
        <a class="button button-secondary"
           href="synapse-191.pdf"
           download>
          Download PDF ↓
        </a>    
      </div>    

      <p class="hero-meta">Joren Heit · 2025 · Web edition</p>
    </div>{hero_diagram()}
  </section>
  <section class="metric-strip"><div><strong>08</strong><span>native BF instructions</span></div><div><strong>24</strong><span>microcode control signals</span></div><div><strong>09</strong><span>fields per Acus macro-cell</span></div><div><strong>02</strong><span>halves of one project story</span></div></section>
  <section class="section-shell story-intro">
    <p class="section-index">01 / The complete story</p>
    <div class="section-heading-row"><h2>One experiment,<br>from copper to compiler.</h2><p>Synapse-191 began as a physical computer that treats Brainfuck as its native instruction set. Acus followed from the next question: what would it take to compile higher-level program structures back down to that machine?</p></div>
    <div class="pathway-grid">
      <a href="{relative_url('/', '/synapse-191/')}" class="pathway-card hardware-path"><span class="pathway-label">Part I · Hardware</span><h3>The Brainfuck Computer</h3><p>Architecture, microcode, breadboard implementation, supporting tools, and runtime results.</p><span class="text-link">Enter the hardware section {arrow()}</span></a>
      <a href="{relative_url('/', '/acus/')}" class="pathway-card software-path"><span class="pathway-label">Part II · Compilation</span><h3>The Acus Backend</h3><p>Program structure, stack frames, tape abstractions, code generation, caching, and optimization.</p><span class="text-link">Enter the Acus section {arrow()}</span></a>
    </div>
  </section>
  <section class="acus-feature"><div class="acus-feature-code"><span>BF Cell</span><b>→</b><span>MacroCell</span><b>→</b><span>Slot</span><b>→</b><span>Frame</span><b>→</b><span>Tape</span></div><div class="acus-feature-copy"><p class="section-index">02 / Acus spotlight</p><h2>A typed compiler backend for generating Brainfuck.</h2><p>Acus supplies the machinery a frontend needs to lower variables, arrays, structs, pointers, functions, recursion, and control flow onto Brainfuck’s bare tape model.</p><a class="text-link" href="{relative_url('/', '/acus/')}">Read the compiler chapters {arrow()}</a></div></section>
  <section class="section-shell reading-map"><p class="section-index">03 / Reading map</p><h2>Browse the web edition</h2><div class="reading-map-grid">{reading_map}</div></section>
</main>"""


def chapter_cards(route: str, pages: list[dict[str, object]], part: str) -> str:
    cards = []
    for page in [p for p in pages if p["part"] == part]:
        cards.append(f"""<a class="chapter-card" href="{relative_url(route, str(page['route']))}"><span class="chapter-card-number">{page['number']}</span><div><h3>{html.escape(str(page['title']))}</h3><p>{html.escape(str(page['description']))}</p><span class="chapter-card-meta">{page['minutes']} min read {arrow()}</span></div></a>""")
    return '<div class="chapter-grid">' + "".join(cards) + "</div>"


def render_landing(route: str, pages: list[dict[str, object]]) -> str:
    data = LANDINGS[route]
    group = [page for page in pages if page["part"] == data["part"]]
    first_route = str(group[0]["route"]) if group else "/"
    github = '<a class="button button-secondary" href="https://github.com/jorenheit/acus">Acus on GitHub ' + arrow() + "</a>" if data["key"] == "acus" else ""
    compiler = ""
    if data["key"] == "acus":
        compiler = """<section class="compiler-model"><p class="section-index">Backend model</p><div class="compiler-model-grid"><div><span>01</span><h2>Structured input</h2><p>Typed expressions, declarations, functions, and explicit control flow.</p></div><div><span>02</span><h2>Tape model</h2><p>Frames, slots, macro-cells, indirect access paths, and materialization.</p></div><div><span>03</span><h2>Brainfuck output</h2><p>Optimized primitive sequences collapsed into canonical BF commands.</p></div></div></section>"""
    return f"""
<main class="landing-page landing-{data['key']}">
  <section class="landing-hero"><div><p class="eyebrow">{html.escape(data['eyebrow'])}</p><h1>{html.escape(data['title'])}</h1><p class="landing-lead">{html.escape(data['lead'])}</p><div class="landing-actions"><a class="button button-primary" href="{relative_url(route, first_route)}">Begin this section {arrow()}</a>{github}</div></div><div class="landing-accent" aria-hidden="true"><span>{html.escape(data['accent'])}</span></div></section>
  {compiler}
  <section class="section-shell chapter-section"><div class="section-heading-row"><div><p class="section-index">Contents</p><h2>{html.escape(data['part'])}</h2></div><p>Each chapter is presented as a standalone, linkable page with its own local table of contents and previous/next navigation.</p></div>{chapter_cards(route, pages, data['part'])}</section>
</main>"""


def render_article(page: dict[str, object], pages: list[dict[str, object]]) -> str:
    route = str(page["route"])
    group = [entry for entry in pages if entry["part"] == page["part"]]
    sidebar_links = "".join(
        f'<a href="{relative_url(route, str(entry["route"]))}"{(" aria-current=\"page\"" if entry["route"] == route else "")}><span>{entry["number"]}</span>{html.escape(str(entry["title"]))}</a>'
        for entry in group
    )
    landing = "/acus/" if page["part"] == "Brainfuck Compilation" else "/synapse-191/" if page["part"] == "The Brainfuck Computer" else "/appendix/" if page["part"] == "Appendix" else "/"
    sidebar = f'<aside class="article-sidebar"><a class="sidebar-part" href="{relative_url(route, landing)}">{html.escape(str(page["part"]))}</a><nav>{sidebar_links}</nav></aside>'
    sections = [s for s in page["sections"] if s["level"] == 2 or len(page["sections"]) < 8][:14]
    toc = "".join(f'<a class="{"toc-subsection" if section["level"] == 3 else ""}" href="#{section["id"]}">{html.escape(section["title"])}</a>' for section in sections)
    toc_html = f'<aside class="article-toc"><p>On this page</p><nav>{toc}</nav></aside>' if toc else ""
    index = pages.index(page)
    previous = pages[index - 1] if index else None
    following = pages[index + 1] if index < len(pages) - 1 else None
    previous_html = f'<a href="{relative_url(route, str(previous["route"]))}"><span>← Previous</span><strong>{html.escape(str(previous["title"]))}</strong></a>' if previous else "<span></span>"
    next_html = f'<a class="pagination-next" href="{relative_url(route, str(following["route"]))}"><span>Next →</span><strong>{html.escape(str(following["title"]))}</strong></a>' if following else "<span></span>"
    return f"""
<main class="article-page"><div class="article-layout">{sidebar}<article class="report-article"><header class="article-header" id="{html.escape(str(page['chapterId']))}"><nav class="breadcrumbs"><a href="{relative_url(route, '/')}">Home</a><span>/</span><span>{html.escape(str(page['part']))}</span></nav><p class="article-kicker">{html.escape(str(page['part']))} · {page['number']}</p><h1>{html.escape(str(page['title']))}</h1><div class="article-meta"><span>{page['minutes']} min read</span><span>Web edition</span></div></header><div id="article-content" class="article-content">{page['html']}</div><nav class="article-pagination">{previous_html}{next_html}</nav></article>{toc_html}</div></main>"""


def write_page(output: Path, route: str, title: str, description: str, content: str, site_url: str) -> None:
    path = route_output_path(output, route)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(document_html(route, title, description, content, site_url), encoding="utf-8")


def write_search_index(output: Path, pages: list[dict[str, object]]) -> None:
    entries = [
        {
            "title": page["title"],
            "path": str(page["route"]).strip("/") + "/",
            "part": page["part"],
            "description": page["description"],
            "text": str(page["searchText"])[:12000],
        }
        for page in pages
    ]
    for route, landing in LANDINGS.items():
        entries.append({"title": landing["title"], "path": route.strip("/") + "/", "part": landing["part"], "description": landing["lead"], "text": landing["lead"]})
    (output / "assets" / "search-index.json").write_text(json.dumps(entries, ensure_ascii=False, separators=(",", ":")), encoding="utf-8")


def write_auxiliary_files(output: Path, routes: list[str], site_url: str) -> None:
    (output / ".nojekyll").write_text("", encoding="utf-8")
    (output / "robots.txt").write_text("User-agent: *\nAllow: /\n", encoding="utf-8")
    if site_url:
        urls = "".join(f"<url><loc>{html.escape(site_url.rstrip('/') + ('/' if route == '/' else route))}</loc></url>" for route in routes)
        (output / "sitemap.xml").write_text(f'<?xml version="1.0" encoding="UTF-8"?><urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">{urls}</urlset>', encoding="utf-8")


def verify_site(output: Path) -> tuple[int, int]:
    html_files = sorted(output.rglob("*.html"))
    ids = {path: set(re.findall(r'\bid="([^"]+)"', read_text(path))) for path in html_files}
    errors: list[str] = []
    checked = 0
    for page in html_files:
        source = read_text(page)
        references = re.findall(r'(?:href|src)="([^"]+)"', source)
        for reference in references:
            if reference.startswith(("http:", "https:", "mailto:", "data:")) or reference == "#":
                continue
            checked += 1
            path_part, _, fragment = reference.partition("#")
            target = page if not path_part else (page.parent / path_part).resolve()
            if path_part.endswith("/") or target.is_dir():
                target = target / "index.html"
            if not target.exists():
                errors.append(f"{page.relative_to(output)} -> missing {reference}")
                continue
            if fragment and target.suffix == ".html" and fragment not in ids.get(target, set()):
                errors.append(f"{page.relative_to(output)} -> missing #{fragment} in {target.relative_to(output)}")
    if errors:
        raise SystemExit("Internal link validation failed:\n" + "\n".join(errors[:50]))
    return len(html_files), checked


def main() -> None:
    args = parse_args()
    source_dir = find_source(args.source)
    output = safe_output_directory(args.output)
    references = parse_references(read_text(source_dir / "references.tex"))
    main_source = read_text(source_dir / "main.tex")
    document = re.search(r"\\begin\{document\}(.*)\\end\{document\}", main_source, flags=re.S)
    if not document:
        raise SystemExit("main.tex has no document environment")
    expanded = expand_includes(document.group(1), source_dir)
    source_label_titles = collect_source_label_titles(expanded)
    prepared, missing_listings = preprocess_latex(expanded, source_dir, references)
    converted = run_pandoc(prepared)
    pages = split_pages(converted)
    distribute_footnotes(pages)
    owners, labels = collect_labels(pages)
    for identifier, route in collect_source_label_owners(prepared).items():
        owners.setdefault(identifier, route)
        labels.setdefault(identifier, "related section")
    labels.update(source_label_titles)
    add_fallback_anchors(pages, owners)
    for page in pages:
        autolink_reference_urls(page)
        rewrite_page_html(page, pages, owners, labels, source_dir)
        deduplicate_page_ids(page)
        refresh_page_metadata(page)

    copy_assets(source_dir, output)
    write_page(output, "/", "Synapse-191 — Building a Brainfuck Computing Environment", "A native Brainfuck computer and the Acus typed compiler backend, presented as a complete technical web edition.", render_home(pages), args.site_url)
    for route, landing in LANDINGS.items():
        write_page(output, route, landing["title"], landing["lead"], render_landing(route, pages), args.site_url)
    for page in pages:
        write_page(output, str(page["route"]), str(page["title"]), str(page["description"]), render_article(page, pages), args.site_url)
    write_search_index(output, pages)
    all_routes = ["/", *LANDINGS.keys(), *[str(page["route"]) for page in pages]]
    write_auxiliary_files(output, all_routes, args.site_url)
    page_count, link_count = verify_site(output)
    print(f"Built {page_count} pages and verified {link_count} internal references.")
    if missing_listings:
        print(f"Note: {len(missing_listings)} external source listings use placeholders in this standalone build.")


if __name__ == "__main__":
    main()
