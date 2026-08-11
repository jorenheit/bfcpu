(() => {
  const rootPrefix = document.documentElement.dataset.rootPrefix || "";
  const searchDialog = document.querySelector("[data-search-dialog]");
  const searchInput = document.querySelector("[data-search-input]");
  const searchResults = document.querySelector("[data-search-results]");
  let searchIndex = null;

  const escapeHtml = (value) => value.replace(/[&<>"']/g, (character) => ({
    "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;",
  })[character]);

  const loadSearch = async () => {
    if (searchIndex) return searchIndex;
    const response = await fetch(`${rootPrefix}assets/search-index.json`);
    searchIndex = await response.json();
    return searchIndex;
  };

  const score = (entry, terms) => {
    const title = entry.title.toLowerCase();
    const part = entry.part.toLowerCase();
    const description = entry.description.toLowerCase();
    const text = entry.text.toLowerCase();
    let total = 0;
    for (const term of terms) {
      if (!text.includes(term) && !title.includes(term) && !part.includes(term)) return -1;
      if (title === term) total += 80;
      else if (title.startsWith(term)) total += 45;
      else if (title.includes(term)) total += 30;
      if (part.includes(term)) total += 12;
      if (description.includes(term)) total += 8;
      if (text.includes(term)) total += 2;
    }
    return total;
  };

  const renderResults = async () => {
    const entries = await loadSearch();
    const terms = searchInput.value.toLowerCase().trim().split(/\s+/).filter(Boolean);
    const results = terms.length
      ? entries.map((entry) => ({ entry, score: score(entry, terms) })).filter((item) => item.score >= 0).sort((a, b) => b.score - a.score).slice(0, 9).map((item) => item.entry)
      : entries.slice(0, 7);
    searchResults.innerHTML = `<p class="search-results-label">${terms.length ? `${results.length} matches` : "Suggested chapters"}</p>` + (results.length
      ? results.map((entry) => `<a class="search-result" href="${rootPrefix}${entry.path}"><span class="search-result-part">${escapeHtml(entry.part)}</span><strong>${escapeHtml(entry.title)}</strong><span>${escapeHtml(entry.description)}</span></a>`).join("")
      : '<p class="search-empty">No matching section. Try a broader term.</p>');
  };

  const openSearch = async () => {
    searchDialog.hidden = false;
    document.documentElement.classList.add("dialog-open");
    await renderResults();
    setTimeout(() => searchInput.focus(), 20);
  };
  const closeSearch = () => {
    searchDialog.hidden = true;
    document.documentElement.classList.remove("dialog-open");
  };
  document.querySelectorAll("[data-search-open]").forEach((button) => button.addEventListener("click", openSearch));
  document.querySelector("[data-search-close]")?.addEventListener("click", closeSearch);
  searchDialog?.addEventListener("mousedown", (event) => { if (event.target === searchDialog) closeSearch(); });
  searchInput?.addEventListener("input", renderResults);
  document.addEventListener("keydown", (event) => {
    const editing = event.target.matches?.("input, textarea, select, [contenteditable='true']");
    if ((event.metaKey || event.ctrlKey) && event.key.toLowerCase() === "k") { event.preventDefault(); openSearch(); }
    else if (event.key === "/" && !editing) { event.preventDefault(); openSearch(); }
    else if (event.key === "Escape") closeSearch();
  });

  const menuButton = document.querySelector("[data-menu-open]");
  const mobileNav = document.querySelector(".mobile-nav");
  menuButton?.addEventListener("click", () => {
    const open = mobileNav.classList.toggle("open");
    menuButton.setAttribute("aria-expanded", String(open));
  });

  const progress = document.querySelector(".reading-progress span");
  if (progress && document.querySelector(".article-page")) {
    let frame = 0;
    const update = () => {
      const height = document.documentElement.scrollHeight - innerHeight;
      progress.style.transform = `scaleX(${height > 0 ? Math.min(1, scrollY / height) : 0})`;
      frame = 0;
    };
    addEventListener("scroll", () => { if (!frame) frame = requestAnimationFrame(update); }, { passive: true });
    addEventListener("resize", update);
    update();
  }

  const article = document.getElementById("article-content");
  article?.querySelectorAll('a[href^="http"]').forEach((anchor) => { anchor.target = "_blank"; anchor.rel = "noreferrer"; });
  article?.querySelectorAll("pre").forEach((pre) => {
    if (pre.parentElement.classList.contains("code-frame")) return;
    const frame = document.createElement("div");
    frame.className = "code-frame";
    pre.parentNode.insertBefore(frame, pre);
    frame.appendChild(pre);
    const button = document.createElement("button");
    button.type = "button";
    button.className = "copy-code";
    button.textContent = "Copy";
    button.addEventListener("click", async () => {
      await navigator.clipboard.writeText(pre.innerText);
      button.textContent = "Copied";
      setTimeout(() => { button.textContent = "Copy"; }, 1200);
    });
    frame.appendChild(button);
  });

  const tocLinks = [...document.querySelectorAll(".article-toc a, .mobile-article-toc a")];
  if (tocLinks.length && "IntersectionObserver" in window) {
    const byId = new Map();
    tocLinks.forEach((link) => {
      const id = decodeURIComponent(link.hash.slice(1));
      byId.set(id, [...(byId.get(id) || []), link]);
    });
    const observer = new IntersectionObserver((entries) => {
      entries.filter((entry) => entry.isIntersecting).forEach((entry) => {
        tocLinks.forEach((link) => link.classList.remove("current"));
        byId.get(entry.target.id)?.forEach((link) => link.classList.add("current"));
      });
    }, { rootMargin: "-20% 0px -70% 0px" });
    byId.forEach((_, id) => { const heading = document.getElementById(id); if (heading) observer.observe(heading); });
  }
})();
