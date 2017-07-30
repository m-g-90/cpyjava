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
    "\tfor x in range (0,100):\n"
    "\t\tp('')\n"
    "t = []\n"
    "for x in range(0,100):\n"
    "\tt.append(threading.Thread(target=javacall))\n"
    "\tt[-1].start()\n"
    "for x in t:\n"
    "\tx.join()\n"

    ,
    "import cpyjava\n"
    "ok = True\n"
    "try:\n"
    "\tcpyjava.packages.java.lang.System.err.print(1,2,3,4,5,6) #expected to fail\n"
    "\tok = False\n"
    "except:\n"
    "\tpass\n"
    "if not ok:\n"
    "\traise Exception('Java Exception was not transfered to python.')"

    ,
    "import cpyjava\n"
    "if not isinstance(cpyjava.packages.java.util.HashMap(),cpyjava.getType('java/util/Map')):\n"
    "\traise Exception('Java Interface inheritance failed.')\n"
    "if not isinstance(cpyjava.packages.java.util.HashMap(),cpyjava.getType('java/util/HashMap')):\n"
    "\traise Exception('Java Class inheritance failed.')\n"
    "if isinstance(cpyjava.packages.java.util.HashMap(),cpyjava.getType('java/util/TreeMap')):\n"
    "\traise Exception('Java Class inheritance failed (false positive).')\n"

    ,
    NULL
};

#endif // SELFTEST_H
