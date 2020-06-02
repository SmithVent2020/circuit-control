#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG_MODE

#define debugPrint(a) Serial.print(a);
#define debugPrintln(a) Serial.println(a);
#define debugPrintVar(lbl,var) Serial.print(lbl);Serial.println(var);

#else

#define debugPrint(a) 
#define debugPrintln(a) 
#define debugPrintVar(lbl,var) 

#endif

#endif