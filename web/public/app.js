/* LinNote site behaviour: live release stats, copy buttons, reveals, ⌘K palette. */
(function () {
  "use strict";
  var reduced = window.matchMedia("(prefers-reduced-motion: reduce)").matches;

  // Only now may the stylesheet hide anything: if this file never runs, the
  // page must still be fully readable.
  document.documentElement.classList.add("js");

  /* ---- live stats from the Worker ---------------------------------- */
  function setText(id, value) {
    var el = document.getElementById(id);
    if (el && value !== null && value !== undefined) el.textContent = value;
  }
  function group(n) { return new Intl.NumberFormat("en-US").format(n); }

  fetch("/api/stats", { headers: { accept: "application/json" } })
    .then(function (r) { return r.ok ? r.json() : null; })
    .then(function (d) {
      if (!d) return;
      if (d.version) {
        setText("statVersion", d.version);
        document.querySelectorAll("[data-version]").forEach(function (el) {
          el.textContent = el.textContent.replace(/\d+\.\d+\.\d+/, d.version);
        });
      }
      if (typeof d.downloads === "number") setText("statDownloads", group(d.downloads));
      if (typeof d.stars === "number") setText("statStars", group(d.stars));
    })
    .catch(function () { /* the chips keep their dash */ });

  /* ---- copy buttons ------------------------------------------------ */
  document.querySelectorAll(".copy").forEach(function (btn) {
    btn.addEventListener("click", function () {
      var text = btn.getAttribute("data-copy") || "";
      var done = function () {
        var old = btn.textContent;
        btn.textContent = "Copied";
        btn.setAttribute("data-state", "success");
        setTimeout(function () {
          btn.textContent = old;
          btn.removeAttribute("data-state");
        }, 1600);
      };
      if (navigator.clipboard && window.isSecureContext) {
        navigator.clipboard.writeText(text).then(done, done);
      } else {
        var ta = document.createElement("textarea");
        ta.value = text; ta.setAttribute("readonly", "");
        ta.style.position = "fixed"; ta.style.opacity = "0";
        document.body.appendChild(ta); ta.select();
        try { document.execCommand("copy"); } catch (e) { /* nothing to do */ }
        document.body.removeChild(ta); done();
      }
    });
  });

  /* ---- reveals ----------------------------------------------------- */
  var rv = document.querySelectorAll(".rv");
  if (reduced || !("IntersectionObserver" in window)) {
    rv.forEach(function (el) { el.classList.add("is-in"); });
  } else {
    var io = new IntersectionObserver(function (entries) {
      entries.forEach(function (e) {
        if (e.isIntersecting) { e.target.classList.add("is-in"); io.unobserve(e.target); }
      });
    }, { rootMargin: "0px 0px -8% 0px" });
    rv.forEach(function (el) { io.observe(el); });
  }

  /* ---- one-shot type-in in the terminal ---------------------------- */
  var typeTarget = document.getElementById("typeLine");
  if (typeTarget) {
    var line = "linnote";
    if (reduced) { typeTarget.textContent = line; }
    else {
      var i = 0;
      var tick = setInterval(function () {
        typeTarget.textContent = line.slice(0, ++i);
        if (i >= line.length) clearInterval(tick);
      }, 95);
    }
  }

  /* ---- mobile menu ------------------------------------------------- */
  var navToggle = document.getElementById("navToggle");
  var navLinks = document.getElementById("navLinks");
  if (navToggle && navLinks) {
    var closeNav = function () {
      navLinks.removeAttribute("data-open");
      navToggle.setAttribute("aria-expanded", "false");
    };
    navToggle.addEventListener("click", function () {
      var open = navLinks.getAttribute("data-open") === "true";
      if (open) { closeNav(); }
      else {
        navLinks.setAttribute("data-open", "true");
        navToggle.setAttribute("aria-expanded", "true");
      }
    });
    // Tapping a destination should close the panel behind it.
    navLinks.addEventListener("click", function (e) {
      if (e.target.tagName === "A") closeNav();
    });
    document.addEventListener("keydown", function (e) {
      if (e.key === "Escape") closeNav();
    });
    document.addEventListener("click", function (e) {
      if (!navLinks.contains(e.target) && e.target !== navToggle && !navToggle.contains(e.target)) closeNav();
    });
  }

  /* ---- ⌘K palette -------------------------------------------------- */
  var ITEMS = [
    { label: "Install", hint: "section", href: "#install" },
    { label: "See it in action", hint: "section", href: "#tour" },
    { label: "Features", hint: "section", href: "#features" },
    { label: "Support the project", hint: "section", href: "#support" },
    { label: "Guides", hint: "guide", href: "/guide/" },
    { label: "Keyboard shortcuts", hint: "guide", href: "/guide/shortcuts" },
    { label: "User guide", hint: "markdown", href: "/USER_GUIDE.md" },
    { label: "Comparison with other apps", hint: "markdown", href: "/COMPARISON.md" },
    { label: "Source on GitHub", hint: "external", href: "https://github.com/sfnemis/linnote" },
    { label: "Releases and downloads", hint: "external", href: "https://github.com/sfnemis/linnote/releases" },
    { label: "Report a bug", hint: "external", href: "https://github.com/sfnemis/linnote/issues" },
    { label: "install.sh", hint: "script", href: "/install.sh" },
    { label: "LinNote for assistants (llms.txt)", hint: "markdown", href: "/llms.txt" }
  ];

  var pal = document.getElementById("pal");
  var palInput = document.getElementById("palInput");
  var palList = document.getElementById("palList");
  var palOpen = document.getElementById("palOpen");
  var cursor = 0, shown = ITEMS.slice(), lastFocus = null;

  function render() {
    palList.innerHTML = "";
    if (!shown.length) {
      var p = document.createElement("li");
      p.className = "pal-empty";
      p.textContent = "Nothing matches that.";
      palList.appendChild(p);
      return;
    }
    shown.forEach(function (item, idx) {
      var li = document.createElement("li");
      li.setAttribute("aria-selected", idx === cursor ? "true" : "false");
      var a = document.createElement("a");
      a.href = item.href;
      a.textContent = item.label;
      var s = document.createElement("span");
      s.textContent = item.hint;
      a.appendChild(s);
      a.addEventListener("click", close);
      li.appendChild(a);
      palList.appendChild(li);
    });
  }
  function filter() {
    var q = palInput.value.trim().toLowerCase();
    shown = q ? ITEMS.filter(function (i) { return i.label.toLowerCase().indexOf(q) !== -1 || i.hint.indexOf(q) !== -1; }) : ITEMS.slice();
    cursor = 0;
    render();
  }
  function open() {
    lastFocus = document.activeElement;
    pal.hidden = false;
    palInput.value = "";
    filter();
    palInput.focus();
  }
  function close() {
    pal.hidden = true;
    if (lastFocus && lastFocus.focus) lastFocus.focus();
  }

  if (palOpen) palOpen.addEventListener("click", open);
  if (palInput) palInput.addEventListener("input", filter);

  document.addEventListener("keydown", function (e) {
    if ((e.metaKey || e.ctrlKey) && e.key.toLowerCase() === "k") {
      e.preventDefault();
      pal.hidden ? open() : close();
      return;
    }
    if (pal.hidden) return;
    if (e.key === "Escape") { e.preventDefault(); close(); return; }
    if (e.key === "ArrowDown" || e.key === "ArrowUp") {
      e.preventDefault();
      if (!shown.length) return;
      cursor = (cursor + (e.key === "ArrowDown" ? 1 : -1) + shown.length) % shown.length;
      render();
      return;
    }
    if (e.key === "Enter" && shown.length) {
      e.preventDefault();
      window.location.href = shown[cursor].href;
      close();
    }
  });
  if (pal) pal.addEventListener("click", function (e) { if (e.target === pal) close(); });
})();

/* Screenshot lightbox. Click a tour screenshot to read it full size, then
   Escape, the backdrop or the close button to leave, and the arrow keys to
   walk the ten shots. */
(function () {
  var lb = document.getElementById("lightbox");
  if (!lb) return;
  var img = document.getElementById("lbImg");
  var title = document.getElementById("lbTitle");
  var count = document.getElementById("lbCount");
  var closeBtn = document.getElementById("lbClose");
  var prevBtn = document.getElementById("lbPrev");
  var nextBtn = document.getElementById("lbNext");

  var shots = Array.prototype.slice.call(document.querySelectorAll(".shot"));
  if (!shots.length) return;

  var index = 0;
  var lastFocus = null;

  function show(i) {
    index = (i + shots.length) % shots.length;
    var source = shots[index].querySelector("img");
    img.src = source.getAttribute("src");
    img.alt = source.getAttribute("alt") || "";
    title.textContent = source.getAttribute("alt") || "";
    count.textContent = (index + 1) + " / " + shots.length;
  }

  function open(i) {
    // Opening while already open would capture the dialog's own close button
    // as the element to restore focus to, so it only moves the image.
    if (!lb.hidden) { show(i); return; }
    lastFocus = document.activeElement;
    show(i);
    lb.hidden = false;
    // The page must not scroll behind the dialog. A class rather than a
    // saved inline value: saving one means a second open can save "hidden"
    // and leave the page unscrollable for good.
    document.body.classList.add("lb-open");
    closeBtn.focus();
  }

  function close() {
    lb.hidden = true;
    document.body.classList.remove("lb-open");
    if (lastFocus && lastFocus.focus) lastFocus.focus();
  }

  shots.forEach(function (btn, i) {
    btn.addEventListener("click", function () { open(i); });
  });

  closeBtn.addEventListener("click", close);
  prevBtn.addEventListener("click", function () { show(index - 1); });
  nextBtn.addEventListener("click", function () { show(index + 1); });

  // A click on the backdrop closes; a click on the image or a control does not.
  lb.addEventListener("click", function (e) {
    if (e.target === lb || e.target.classList.contains("lb-fig")) close();
  });

  document.addEventListener("keydown", function (e) {
    if (lb.hidden) return;
    if (e.key === "Escape") { e.preventDefault(); close(); return; }
    if (e.key === "ArrowLeft") { e.preventDefault(); show(index - 1); return; }
    if (e.key === "ArrowRight") { e.preventDefault(); show(index + 1); return; }
    if (e.key === "Tab") {
      // Keep the focus inside the dialog.
      var order = [closeBtn, prevBtn, nextBtn];
      var at = order.indexOf(document.activeElement);
      e.preventDefault();
      order[(at + (e.shiftKey ? -1 : 1) + order.length) % order.length].focus();
    }
  });
})();
