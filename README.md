python extension to use java objects.

Features:
  - supports 'isinstanceof'
  - conversion from/to java primitive types (excluding arrays) and java.lang.String
  - Any other class/object is wrapped and supported via reflection 
  - "brute force" approach to handle function overloading: functions gets selected if the arguments can be converted to the required types
  
Passing python objects to java is currently not planned.

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
