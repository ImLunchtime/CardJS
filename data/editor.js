var editorState = "fileList";
var files = [];
var selectedIndex = 0;
var lastKeyTime = 0;
var debounceDelay = 150;
var currentPath = "";
var lines = [];
var cursorRow = 0;
var cursorCol = 0;
var topLine = 0;
var lineHeight = 16;
var charWidth = 12;
var textStartY = 30;
var marginX = 4;
var firstVisibleCol = 0;

function loadSpiffsJsFiles() {
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
        var base = name;
        if (base.charAt(0) === "/") {
            base = base.substring(1);
        }
        if (base === "boot.js" || base === "menu.js") {
            continue;
        }
        files.push(name);
    }
    files.sort();
}

function drawFileList() {
    var bg = color(0, 0, 0);
    var fg = color(255, 255, 255);
    drawFillRect(0, 0, width(), height(), bg);
    setTextSize(2);
    setTextColor(fg);
    drawString("JS Editor", marginX, 4);
    var items = ["New File"];
    for (var i = 0; i < files.length; i++) {
        var display = files[i];
        if (display.charAt(0) === "/") {
            display = display.substring(1);
        }
        items.push(display);
    }
    var startY = textStartY;
    for (var j = 0; j < items.length; j++) {
        var prefix = (j === selectedIndex) ? "> " : "  ";
        drawString(prefix + items[j], marginX, startY + j * lineHeight);
    }
}

function ensureCursorVisible() {
    var maxLines = Math.floor((height() - textStartY) / lineHeight);
    if (cursorRow < 0) {
        cursorRow = 0;
    }
    if (cursorRow >= lines.length) {
        cursorRow = lines.length - 1;
        if (cursorRow < 0) {
            cursorRow = 0;
        }
    }
    if (cursorRow < topLine) {
        topLine = cursorRow;
    }
    if (cursorRow >= topLine + maxLines) {
        topLine = cursorRow - maxLines + 1;
    }
    if (cursorRow < 0) {
        topLine = 0;
    }
    var line = lines.length > 0 ? lines[cursorRow] : "";
    if (cursorCol < 0) {
        cursorCol = 0;
    }
    if (cursorCol > line.length) {
        cursorCol = line.length;
    }
    var visibleWidth = width() - marginX;
    var maxCols = Math.floor(visibleWidth / charWidth);
    if (maxCols < 1) {
        maxCols = 1;
    }
    if (cursorCol < firstVisibleCol) {
        firstVisibleCol = cursorCol;
    }
    if (cursorCol >= firstVisibleCol + maxCols) {
        firstVisibleCol = cursorCol - maxCols + 1;
    }
    if (firstVisibleCol < 0) {
        firstVisibleCol = 0;
    }
}

function drawEditorScreen() {
    var bg = color(0, 0, 0);
    var fg = color(255, 255, 255);
    drawFillRect(0, 0, width(), height(), bg);
    setTextSize(2);
    setTextColor(fg);
    var nameDisplay = currentPath;
    if (nameDisplay.charAt(0) === "/") {
        nameDisplay = nameDisplay.substring(1);
    }
    drawString(nameDisplay, marginX, 4);
    setTextSize(1);
    drawString("[Fn+Enter]Save", marginX, 6 + lineHeight);
    setTextSize(2);
    var maxLines = Math.floor((height() - textStartY) / lineHeight);
    var visibleWidth = width() - marginX;
    var maxCols = Math.floor(visibleWidth / charWidth);
    if (maxCols < 1) {
        maxCols = 1;
    }
    var y = textStartY;
    for (var i = 0; i < maxLines; i++) {
        var idx = topLine + i;
        if (idx >= lines.length) {
            break;
        }
        var text = lines[idx];
        var startCol = firstVisibleCol;
        if (startCol < 0) {
            startCol = 0;
        }
        if (startCol > text.length) {
            startCol = text.length;
        }
        var visible = text.substring(startCol, startCol + maxCols);
        drawString(visible, marginX, y);
        y += lineHeight;
    }
    var cursorScreenRow = cursorRow - topLine;
    if (cursorScreenRow >= 0 && cursorScreenRow < maxLines) {
        var cx = marginX + (cursorCol - firstVisibleCol) * charWidth;
        var cy = textStartY + cursorScreenRow * lineHeight;
        drawFillRect(cx, cy, 2, lineHeight, fg);
    }
}

function drawNameInput(buffer) {
    var bg = color(0, 0, 0);
    var fg = color(255, 255, 255);
    drawFillRect(0, 0, width(), height(), bg);
    setTextSize(2);
    setTextColor(fg);
    drawString("New file name", marginX, 4);
    drawString(buffer, marginX, textStartY);
}

function splitTextToLines(text) {
    var result = [];
    var start = 0;
    for (var i = 0; i < text.length; i++) {
        if (text.charAt(i) === "\n") {
            result.push(text.substring(start, i));
            start = i + 1;
        }
    }
    result.push(text.substring(start));
    return result;
}

function joinLinesToText() {
    var s = "";
    for (var i = 0; i < lines.length; i++) {
        s += lines[i];
        if (i !== lines.length - 1) {
            s += "\n";
        }
    }
    return s;
}

function loadFileToEditor(path) {
    currentPath = path;
    var content = readFileSpiffs(path);
    lines = splitTextToLines(content);
    if (lines.length === 0) {
        lines.push("");
    }
    cursorRow = 0;
    cursorCol = 0;
    topLine = 0;
    firstVisibleCol = 0;
    ensureCursorVisible();
    drawEditorScreen();
}

function saveCurrentFile() {
    var text = joinLinesToText();
    writeFileSpiffs(currentPath, text);
}

function moveCursorLeft() {
    if (cursorCol > 0) {
        cursorCol--;
        return;
    }
    if (cursorRow > 0) {
        cursorRow--;
        var prevLine = lines[cursorRow];
        cursorCol = prevLine.length;
    }
}

function moveCursorRight() {
    var line = lines[cursorRow];
    if (cursorCol < line.length) {
        cursorCol++;
        return;
    }
    if (cursorRow < lines.length - 1) {
        cursorRow++;
        cursorCol = 0;
    }
}

function moveCursorUp() {
    if (cursorRow > 0) {
        cursorRow--;
        var line = lines[cursorRow];
        if (cursorCol > line.length) {
            cursorCol = line.length;
        }
    }
}

function moveCursorDown() {
    if (cursorRow < lines.length - 1) {
        cursorRow++;
        var line = lines[cursorRow];
        if (cursorCol > line.length) {
            cursorCol = line.length;
        }
    }
}

function insertCharacter(ch) {
    var line = lines[cursorRow];
    var before = line.substring(0, cursorCol);
    var after = line.substring(cursorCol);
    lines[cursorRow] = before + ch + after;
    cursorCol++;
}

function insertNewline() {
    var line = lines[cursorRow];
    var before = line.substring(0, cursorCol);
    var after = line.substring(cursorCol);
    lines[cursorRow] = before;
    lines.splice(cursorRow + 1, 0, after);
    cursorRow++;
    cursorCol = 0;
}

function backspace() {
    if (cursorCol > 0) {
        var line = lines[cursorRow];
        var before = line.substring(0, cursorCol - 1);
        var after = line.substring(cursorCol);
        lines[cursorRow] = before + after;
        cursorCol--;
        return;
    }
    if (cursorRow === 0) {
        return;
    }
    var current = lines[cursorRow];
    var prev = lines[cursorRow - 1];
    cursorCol = prev.length;
    lines[cursorRow - 1] = prev + current;
    lines.splice(cursorRow, 1);
    cursorRow--;
}

function parseArrowFromState(state) {
    var result = {
        left: false,
        right: false,
        up: false,
        down: false
    };
    if (!state.fn) {
        return result;
    }
    var keys = state.keys;
    for (var i = 0; i < keys.length; i++) {
        var k = keys[i];
        if (k === ";") {
            result.up = true;
        } else if (k === ".") {
            result.down = true;
        } else if (k === ",") {
            result.left = true;
        } else if (k === "/") {
            result.right = true;
        }
    }
    return result;
}

function handleFileListInput(state) {
    var arrows = parseArrowFromState(state);
    var itemCount = files.length + 1;
    if (arrows.up) {
        selectedIndex = (selectedIndex - 1 + itemCount) % itemCount;
        drawFileList();
        return;
    }
    if (arrows.down) {
        selectedIndex = (selectedIndex + 1) % itemCount;
        drawFileList();
        return;
    }
    if (state.enter && !state.fn) {
        if (selectedIndex === 0) {
            editorState = "nameInput";
            nameBuffer = "";
            drawNameInput(nameBuffer);
        } else {
            var path = "/" + files[selectedIndex - 1];
            editorState = "editor";
            loadFileToEditor(path);
        }
    }
}

var nameBuffer = "";

function handleNameInput(state) {
    var keys = state.keys;
    if (state.del) {
        if (nameBuffer.length > 0) {
            nameBuffer = nameBuffer.substring(0, nameBuffer.length - 1);
        }
        drawNameInput(nameBuffer);
        return;
    }
    if (state.enter && !state.fn) {
        if (nameBuffer.length === 0) {
            return;
        }
        var filename = nameBuffer;
        if (filename.indexOf(".") === -1) {
            filename = filename + ".js";
        }
        if (filename.charAt(0) !== "/") {
            filename = "/" + filename;
        }
        currentPath = filename;
        writeFileSpiffs(currentPath, "");
        loadSpiffsJsFiles();
        editorState = "editor";
        lines = [""];
        cursorRow = 0;
        cursorCol = 0;
        topLine = 0;
        firstVisibleCol = 0;
        drawEditorScreen();
        return;
    }
    if (state.enter && state.fn) {
        editorState = "fileList";
        drawFileList();
        return;
    }
    for (var i = 0; i < keys.length; i++) {
        var k = keys[i];
        if (k === ";" || k === "." || k === "," || k === "/") {
            continue;
        }
        nameBuffer += k;
    }
    drawNameInput(nameBuffer);
}

function handleEditorInput(state) {
    var arrows = parseArrowFromState(state);
    if (arrows.left) {
        moveCursorLeft();
    } else if (arrows.right) {
        moveCursorRight();
    } else if (arrows.up) {
        moveCursorUp();
    } else if (arrows.down) {
        moveCursorDown();
    } else if (state.del && !state.fn) {
        backspace();
    } else if (state.enter && !state.fn) {
        insertNewline();
    } else if (state.enter && state.fn) {
        saveCurrentFile();
        editorState = "fileList";
        loadSpiffsJsFiles();
        selectedIndex = 0;
        drawFileList();
        return;
    } else {
        var keys = state.keys;
        for (var i = 0; i < keys.length; i++) {
            var k = keys[i];
            if (state.fn && (k === ";" || k === "." || k === "," || k === "/")) {
                continue;
            }
            insertCharacter(k);
        }
    }
    ensureCursorVisible();
    drawEditorScreen();
}

function setup() {
    loadSpiffsJsFiles();
    editorState = "fileList";
    selectedIndex = 0;
    drawFileList();
}

function loop() {
    var currentTime = now();
    if (currentTime - lastKeyTime < debounceDelay) {
        return;
    }
    var state = getKeyStatus();
    if (state.keys.length === 0 && !state.del && !state.enter && !state.up && !state.down && !state.tab && !state.fn) {
        return;
    }
    lastKeyTime = currentTime;
    if (editorState === "fileList") {
        handleFileListInput(state);
    } else if (editorState === "nameInput") {
        handleNameInput(state);
    } else if (editorState === "editor") {
        handleEditorInput(state);
    }
}
