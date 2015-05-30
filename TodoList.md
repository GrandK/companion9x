# General #
  * Add a what's new information to be displayed while prompting for updates
  * Add what's new for firmware updates
  * Support for higher voltages in telemetry screen and in custom switches
  * ~~Display somewhere the current firmware chosen, so that it is always visible~~
  * ~~Display the available size on the EEPROM~~
  * ~~Current model displayed in bold in the MDI child windows~~
  * ~~Better look & feel for the simu layout. These big buttons are not so beautiful!~~
  * ~~Models configuration print~~
  * ~~Curves printed as graphics~~
  * ~~Gruvin9x-stable simulation~~
  * ~~Try to catch exceptions from external firmwares and display them as errors (rather than exiting with a SEGFAULT + core dumped)~~
  * ~~Design a new dialog box with a simple progress bar, based on avrdude output interpretation. With a details hide / show button which shows what we have today~~
  * ~~Recent documents easier to access~~
  * ~~Implement a correct handling of Tx modes so that it's possible to change the mode without messing everything (same as g9x [issue 63](https://code.google.com/p/companion9x/issues/detail?id=63))~~
  * ~~Checking updates and download of selected firmware.~~
  * ~~Add an entry in the contextual menu to change the current model~~
  * ~~Change the icon which comes from eePe (a graphist is welcome!!)~~
  * ~~Splash window should be visible at least 2-3 seconds!~~
  * ~~Phase names visible on simulation~~
  * ~~Beeps not handled yet~~
  * ~~Use the Tx as an input for moving the sticks (use joystick interface USB/SMARTPROPOPLUS)~~
  * UNDO command for cut & paste ~~alternatively a confirm dialog for potentially destructive actions.... (paste on non empty model, delete)~~
  * In simulator: highlight real channels to differentiate them from virtual ones
  * In Model config Wizard drag-drop of wing surfaces to channels
  * Print or save Wizard config screen

# Ideas from eePe #
  * ~~Diff feature to compare two models~~
  * Solid bars for simulators instead of standard sliders (perhaps only css)

# Import / export #
  * One model data import / export
  * Make our own .c9x files based on an xml formalism defined in the xsd
  * ~~Er9x export (one TODO remaining) / simulation~~
  * ~~Th9x import / export / simulation~~

# Multi platforms #
  * ~~Mac packaging~~
  * ~~Debian / Ubuntu packaging~~
  * RedHat / Suse packaging