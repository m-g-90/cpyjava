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
    "import cpyjava\n"
    "jobject = cpyjava.packages.java.lang.System.out\n"
    "ok = True\n"
    "try:\n"
    "\tjobject('test')\n"
    "\tok=False\n"
    "except:\n"
    "\tpass\n"
    "if not ok:\n"
    "\traise Exception(\"invalid java object call didn't raise an exception\")\n"
    "cpyjava.setCallDecorator(type(jobject),lambda s,d,a: None)\n"
    "try:\n"
    "\tjobject('test')\n"
    "except:\n"
    "\traise Exception('call decorator was not used')\n"

    ,
    "import cpyjava\n"
    "jobject = cpyjava.packages.java.lang.System.out\n"
    "ok = True\n"
    "try:\n"
    "\tjobject.doesnt_exist\n"
    "\tok=False\n"
    "except:\n"
    "\tpass\n"
    "if not ok:\n"
    "\traise Exception(\"invalid java field access didn't raise an exception\")\n"
    "cpyjava.setGetterDecorator(type(jobject),lambda s,d,a: 29384)\n"
    "test = None\n"
    "try:\n"
    "\ttest = jobject.doesnt_exist\n"
    "except:\n"
    "\traise Exception('getter decorator was not used')\n"
    "if test != 29384:\n"
    "   raise Exception('getter decorator return value lost')"

    ,
    "import cpyjava\n"
    "jobject = cpyjava.packages.java.lang.System.out\n"
    "ok = True\n"
    "try:\n"
    "\tjobject.doesnt_exist = 34534\n"
    "\tok=False\n"
    "except:\n"
    "\tpass\n"
    "if not ok:\n"
    "\traise Exception(\"invalid java field access didn't raise an exception\")\n"
    "test = None\n"
    "def s(*args):\n"
    "\tglobal test\n"
    "\ttest = args[3]\n"
    "cpyjava.setSetterDecorator(type(jobject),s)\n"
    "try:\n"
    "\tjobject.doesnt_exist = 34534\n"
    "except:\n"
    "\traise Exception('setter decorator was not used')\n"
    "if test != 34534:\n"
    "   raise Exception('setter decorator return value lost')"

    ,
    "import cpyjava\n"
    "list = cpyjava.packages.java.util.LinkedList()\n"
    "list.add(\"1\")\n"
    "list.add(\"2\")\n"
    "list.add(\"3\")\n"
    "if repr(list) != '[1, 2, 3]':\n"
    "\traise Exception('unexpected list behaviour. '+repr(list))\n"
    "if len(list.toArray()) != list.size():\n"
    "\traise Exception('unexpected list/array behaviour. ['+str(len(list.toArray()))+','+str(list.size())+']')\n"



    ,
    NULL
};

#endif // SELFTEST_H
