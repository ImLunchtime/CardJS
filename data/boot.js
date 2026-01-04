function setup() {
    setTextSize(1);
    setTextColor(color(255, 255, 255));
    drawString("Boot finished! Press Enter for alt.js", 10, 10);
}

function loop() {
    var keys = getKeysPressed();
    if (keys.length > 0 && keys[0] === "Enter") {
        changeScriptSpiffs("/alt.js");
    }
}
