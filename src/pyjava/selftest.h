#ifndef SELFTEST_H
#define SELFTEST_H

static const char * pyjava_selftests[] = {
    "import cpyjava\n"
    "print = cpyjava.packages.java.lang.System.err.print\n"
    "for x in range(0,20):\n"
    "\tprint('')\n"
    "used = cpyjava.memstat('used')\n"
    "for x in range(0,2000):\n"
    "\tprint('')\n"
    "if used != cpyjava.memstat('used'):\n"
    "\tprint('ERROR: memory leak detected;'+str(used)+','+str(cpyjava.memstat('used')))\n"
    "\traise Exception('memory leak detected')\n"

    ,
    "import threading\n"
    "def javacall():\n"
    "\timport cpyjava\n"
    "\tp = cpyjava.packages.java.lang.System.out.print\n"
    "\tfor x in range (0,1000):\n"
    "\t\tp('')\n"
    "t = []\n"
    "for x in range(0,100):\n"
    "\tt.append(threading.Thread(target=javacall))\n"
    "\tt[-1].start()\n"
    "for x in t:\n"
    "\tx.join()\n"



    ,
    "import cpyjava\n"
    "cpyjava\n"

    ,
    NULL
};

#endif // SELFTEST_H
