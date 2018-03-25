[![tag](https://img.shields.io/github/tag/m-g-90/cpyjava.svg)](https://github.com/m-g-90/cpyjava/releases) [![license](https://img.shields.io/github/license/m-g-90/cpyjava.svg)](https://github.com/m-g-90/cpyjava/blob/master/License.md)

[![Build Status](https://travis-ci.org/m-g-90/cpyjava.svg?branch=master)](https://travis-ci.org/m-g-90/cpyjava) [![Build status](https://ci.appveyor.com/api/projects/status/ce7afnnx892q5rl0/branch/master?svg=true)](https://ci.appveyor.com/project/m-g-90/cpyjava/branch/master) 

[![codecov](https://codecov.io/gh/m-g-90/cpyjava/branch/master/graph/badge.svg)](https://codecov.io/gh/m-g-90/cpyjava)  [![Codacy Badge](https://api.codacy.com/project/badge/Grade/2410def413924018a31c088d65ff5e5e)](https://www.codacy.com/app/m-g-90/cpyjava?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=m-g-90/cpyjava&amp;utm_campaign=Badge_Grade) ![](https://scan.coverity.com/projects/13312/badge.svg)


cpyjava is a python extension/module to enable use of java objects from python (cpython). This extension uses JNI to interface with java objects. 

cpyjava is available via pip. Please note that at this point in time, no prebuild wheel is available. See [here](https://wiki.python.org/moin/WindowsCompilers) for Windows compilers.
   
    pip install cpyjava
    
    
Alternatives/Similar Projects:
  - JPype also allows cpython to interface with java objects via JNI, however it has the issue that object classes are referenced by name (not by the class object) which leads to issues in more complicated class loader structures.
  - Py4j is an python module that uses sockets (not JNI) to communicate between python and java.

Features:
  - supports 'isinstanceof'
  - conversion from/to java primitive types and java.lang.String
  - java map,list,iterable/iterator and arrays can be used like the equivalent python objects
  - Any other class/object is wrapped and supported via reflection 
  - "brute force" approach to handle function overloading: functions gets selected if the arguments can be converted to the required types
  - decorators to customize getter,setter and call functionality
  - function to register an object from java in a python dictionary. This is helpfull to e.g. pass classes of a non-system class loader to python
  
Passing python objects to java is currently not planned. Passing python objects that are wrapped java objects is of course possible.

This module is written exclusively in C. You can use the setup.py to install the module or use the executable project to test this library. Alternatively, this library can be added easily to a project with an embedded python interpreter. Java is required to run this. In case setup.py is used, Java must be installed before setup.py is executed.

Note: the project currently uses a .pro file (qmake/Qt), but doesn't require/use Qt.

Examples:


  Use System.out.print:
    
    import cpyjava
    cpyjava.packages.java.lang.System.out.println('Python to Java print call')
    #alternatively
    java = cpyjava.packages.java
    java.lang.System.out.println("Python to Java print call") #this line works here and in java
    
    
 Use a java Map (including creation of an instance of a Java Class):
    
    import cpyjava
    map = cpyjava.packages.java.util.HashMap() # calls the java constructor
    map.put("key","value") 
    repr(map)
    print(map["key"])
    
 Accelerate interaction with java by keeping the current thread attached to the jvm:
    
    import cpyjava
    with cpyjava.env:
      #some code that interacts with java objects
      pass
      
 Creating a "synchronized" block in python on a java object:
    
    import cpyjava
    map = cpyjava.packages.java.util.HashMap() # create a java object
    with cpyjava.synchronized(map):
      map.put("key",1)
      
  
Compiling the Project:
  - Open the cpyjava.pro project in qtcreator
  - Adapt the java and python paths/versions 
  
  or
  
  - pip install cpyjava
   
