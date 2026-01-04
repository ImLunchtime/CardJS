var files = [];
var selectedIndex = 0;
var lastKeyTime = 0;
var debounceDelay = 200;

function loadFiles() {
    files = [];
    var all = listFilesSpiffs("/");
    for (var i = 0; i < all.length; i++) {
        var name = all[i];
        if (name.length < 4) {
            continue;
        }
        if (name.substr(name.length - 3) !== ".js") {
            continue;
        }
        if (name === "/boot.js" || name === "/menu.js") {
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

    if (files.length === 0) {
        drawString("No scripts found", 10, 10);
        return;
    }

    drawString("Select script:", 10, 10);

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
    if (!state.up && !state.down && !state.enter) {
        return;
    }
    var changed = false;

    if (files.length > 0) {
        if (state.up) {
            selectedIndex = (selectedIndex - 1 + files.length) % files.length;
            changed = true;
        } else if (state.down) {
            selectedIndex = (selectedIndex + 1) % files.length;
            changed = true;
        } else if (state.enter) {
            changeScriptSpiffs("/" + files[selectedIndex]);
            lastKeyTime = currentTime;
            return;
        }
    }

    if (changed) {
        drawMenu();
        lastKeyTime = currentTime;
    }
}
