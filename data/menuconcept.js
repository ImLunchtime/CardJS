var menu = Menu();

function buttonClicked(){
    print("Button Clicked");
}

function setup(){
    menu.addButtonItem("Reboot", buttonClicked);
    menu.addNumberItem("Brightness", 50, 0, 100);
    menu.addStringItem("Name", 32, "");

    var selections = ["Option 1", "Option 2", "Option 3"];
    menu.addSelectionItem("Option", selections, 0);
    menu.addBoolItem("LoRa", false, "Enabled", "Disabled");
}
function loop(){
    menu.update();
}
