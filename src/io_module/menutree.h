namespace MenuTree {
  MenuItem clearItem("Clear", [](MenuItem*, LCDBuffer &buffer, LCDScreen &) -> MenuItem* {
    buffer.clear();
    return nullptr;
  });

  MenuItem echoOnItem("On", [](MenuItem *self, LCDBuffer &buffer, LCDScreen &) -> MenuItem* {
    buffer.setEchoEnabled(true);
    return self->getRoot();
  });

  MenuItem echoOffItem("Off", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setEchoEnabled(false);
    return self->getRoot();
  });

  MenuItem autoscrollOnItem("On", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setAutoScroll(true);
    return self->getRoot();
  });

  MenuItem autoscrollOffItem("Off", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setAutoScroll(false);
    return self->getRoot();
  });

  MenuItem hexModeItem("Hexadecimal", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setMode(HEXADECIMAL);
    return self->getRoot();
  });

  MenuItem decModeItem("Decimal", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setMode(DECIMAL);
    return self->getRoot();
  });

  MenuItem textModeItem("Text", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setMode(ASCII);
    return self->getRoot();
  });

  MenuItem exitItem("Exit", [](MenuItem*, LCDBuffer&, LCDScreen &) -> MenuItem* {
    return nullptr;
  });

  MenuItem echoMenu("Echo", echoOnItem, echoOffItem);
  MenuItem autoscrollMenu("Autoscroll", autoscrollOnItem, autoscrollOffItem);
  MenuItem modeMenu("Display Mode", textModeItem, decModeItem, hexModeItem);
  MenuItem rootMenu("Main Menu", clearItem, echoMenu, autoscrollMenu, modeMenu, exitItem);
}