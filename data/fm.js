function setup() {
    setTextSize(2);
    setTextColor(color(255, 255, 255));
    drawString("400.185 00", 17, 15);
    drawRect(7, 10, 160, 70, color(255, 255, 255));
    drawRect(172, 10, 64, 55, color(255, 255, 255));
    drawRect(172, 71, 63, 57, color(255, 255, 255));
    drawRect(7, 85, 159, 40, color(255, 255, 255));
    setTextSize(2);
    setTextColor(color(255, 255, 255));
    drawString("635.125 00", 17, 39);
    setTextSize(1);
    setTextColor(color(255, 255, 255));
    drawString("-n db", 102, 21);
    setTextSize(1);
    setTextColor(color(0, 255, 42));
    drawString("-38 db RX", 102, 44);
    setTextSize(1);
    setTextColor(color(17, 255, 0));
    drawString("===========", 18, 62);
    setTextSize(1);
    setTextColor(color(255, 149, 0));
    drawString("===", 94, 62);
    setTextSize(1);
    setTextColor(color(255, 0, 0));
    drawString("==", 115, 62);
    setTextSize(1);
    setTextColor(color(255, 255, 255));
    drawString("Protocol: Normal UHF FM Mic", 14, 92);
    setTextSize(1);
    setTextColor(color(255, 255, 255));
    drawString("Listening...", 15, 106);
    setTextSize(1);
    setTextColor(color(255, 255, 255));
    drawString("[F] TX Start", 180, 16);
    setTextSize(1);
    setTextColor(color(255, 255, 255));
    drawString("[T] Mute", 180, 32);
    setTextSize(1);
    setTextColor(color(255, 255, 255));
    drawString("[+] [-] Vol", 179, 77);
}

function loop() {
}
