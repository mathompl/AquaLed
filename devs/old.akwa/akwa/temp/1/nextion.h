
char nxHourString [] = "hour";
NexText nxHour = NexText(0, 5, nxHourString);

char snxTempWater [] = "wt";
NexText nxTempWater = NexText(0, 6, snxTempWater);

char snxTempLed [] = "lt";
NexText nxTempLed = NexText(0, 8, snxTempLed);

char snxWaterFans [] = "wf";
NexText nxWaterFans = NexText(0, 26, snxWaterFans);

char snxLedFans [] = "lf";
NexText nxLedFans = NexText(0, 24, snxLedFans);

char sbo [] = "bo";
NexButton offButton = NexButton(0, 23, sbo);

char sba [] = "ba";
NexButton ambientButton = NexButton(0, 26, sba);

char sbn [] = "bn";
NexButton nightButton = NexButton(0, 24, sbn);

char sml [] = "ml";
NexHotspot configButton = NexHotspot(0, 25, sml);

char shm [] = "home";
NexPage home = NexPage(0, 0, shm);

char scfg [] = "config";
NexPage config = NexPage(0, 1, scfg);

char stm [] = "time";
NexPage settime = NexPage(0, 2, stm);

NexButton configBackButton = NexButton(1, 11, "b8");
NexButton configTimeButton = NexButton(1, 7, "b6");
NexButton configOtherButton = NexButton(1, 10, "b7");

NexButton configLED1Button = NexButton(1, 1, "b0");
NexButton configLED2Button = NexButton(1, 2, "b1");
NexButton configLED3Button = NexButton(1, 3, "b2");
NexButton configLED4Button = NexButton(1, 4, "b3");
NexButton configLED5Button = NexButton(1, 6, "b5");
NexButton configLED6Button = NexButton(1, 5, "b4");

NexButton timeCancelButton = NexButton(2, 5, "b1");
NexButton timeSaveButton = NexButton(2, 4, "b0");

NexText ledTexts[] = 
{
  NexText(0, 10, "l1"),
  NexText(0, 12, "l2"),
  NexText(0, 14, "l3"),
  NexText(0, 16, "l4"),
  NexText(0, 18, "l5"),
  NexText(0, 20, "l6")
}; 



