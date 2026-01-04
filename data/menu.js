var files = [];
var selectedIndex = 0;
var lastKeyTime = 0;
var debounceDelay = 200;
var useSd = false;

function loadFiles() {
    files = [];
    var all;
    if (useSd) {
        all = listFilesSD("/");
    } else {
        all = listFilesSpiffs("/");
    }
    for (var i = 0; i < all.length; i++) {
        var name = all[i];
        var base = name;
        if (base.charAt(0) === "/") {
            base = base.substring(1);
        }
        if (base.length < 4) {
            continue;
        }
        if (base.substr(base.length - 3) !== ".js") {
            continue;
        }
        if (base === "boot.js" || base === "menu.js") {
            continue;
        }
        files.push(name);
    }
    if (selectedIndex >= files.length) {
        selectedIndex = 0;
    }
}

function drawMenu() {
    var bg = color(0, 0, 0);
    var fg = color(255, 255, 255);
    drawFillRect(0, 0, width(), height(), bg);
    setTextSize(2);
    setTextColor(fg);

    var storageLabel = useSd ? "SD" : "SPIFFS";
    if (files.length === 0) {
        drawString("No scripts in " + storageLabel, 10, 10);
        setTextSize(1);
        drawString("[Tab] Change Storage SPIFFS/SD", 10, height() - 12);
        return;
    }
    setTextSize(1);
    drawString("Reading from " + storageLabel + ":", 10, 10);
    setTextSize(2);

    var startY = 30;
    var lineHeight = 16;
    for (var i = 0; i < files.length; i++) {
        var display = files[i];
        if (display.charAt(0) === "/") {
            display = display.substring(1);
        }
        var prefix = (i === selectedIndex) ? "> " : "  ";
        drawString(prefix + display, 10, startY + i * lineHeight);
    }
    setTextSize(1);
    drawString("[Tab] Change Storage SPIFFS/SD", 10, height() - 12);
}

function hasKey(keys, key) {
    for (var i = 0; i < keys.length; i++) {
        if (keys[i] === key) {
            return true;
        }
    }
    return false;
}

function setup() {
    loadFiles();
    drawMenu();
}

function loop() {
    var currentTime = now();
    if (currentTime - lastKeyTime < debounceDelay) {
        return;
    }

    var state = getKeyStatus();
    if (!state.up && !state.down && !state.enter && !state.tab) {
        return;
    }
    var changed = false;

    if (state.tab) {
        useSd = !useSd;
        loadFiles();
        selectedIndex = 0;
        changed = true;
    } else if (files.length > 0) {
        if (state.up) {
            selectedIndex = (selectedIndex - 1 + files.length) % files.length;
            changed = true;
        } else if (state.down) {
            selectedIndex = (selectedIndex + 1) % files.length;
            changed = true;
        } else if (state.enter) {
            if (useSd) {
                changeScriptSD("/" + files[selectedIndex]);
            } else {
                changeScriptSpiffs("/" + files[selectedIndex]);
            }
            lastKeyTime = currentTime;
            return;
        }
    }

    if (changed) {
        drawMenu();
        lastKeyTime = currentTime;
    }
}
