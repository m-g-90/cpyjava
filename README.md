![](https://img.shields.io/github/tag/m-g-90/cpyjava.svg) ![](https://img.shields.io/github/license/m-g-90/cpyjava.svg)

![](https://travis-ci.org/m-g-90/cpyjava.svg?branch=master) [![Build status](https://ci.appveyor.com/api/projects/status/ce7afnnx892q5rl0/branch/master?svg=true)](https://ci.appveyor.com/project/m-g-90/cpyjava/branch/master) ![](https://scan.coverity.com/projects/13312/badge.svg)

[![codecov](https://codecov.io/gh/m-g-90/cpyjava/branch/master/graph/badge.svg)](https://codecov.io/gh/m-g-90/cpyjava)  [![Codacy Badge](https://api.codacy.com/project/badge/Grade/2410def413924018a31c088d65ff5e5e)](https://www.codacy.com/app/m-g-90/cpyjava?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=m-g-90/cpyjava&amp;utm_campaign=Badge_Grade)


python extension to use java objects.

Features:
  - supports 'isinstanceof'
  - conversion from/to java primitive types (for now excluding arrays) and java.lang.String
  - Any other class/object is wrapped and supported via reflection 
  - "brute force" approach to handle function overloading: functions gets selected if the arguments can be converted to the required types
  - decorators to customize getter,setter and call functionality
  - function to register an object from java in a python dictionary. This is helpfull to e.g. pass classes of a non-system class loader to python
  
Passing python objects to java is currently not planned. Passing python objects that are wrapped java objects is of course possible.

This module is written exclusively in C. The build system for python modules is not yet implemented. Use the executable project to test this library. Alternatively, this library can be added easily to a project with an embedded python interpreter. 

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
    map.put("key","value") # NOTE: due to currently limited conversion support, only strings can be used
    repr(map)
    print(map["key"])

  
  Compiling the Project:
    1. Open the cpyjava.pro project in qtcreator
    2. Adapt the java and python paths/versions 
   
