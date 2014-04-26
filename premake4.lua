------------------------------------------------
------------  Solution   -----------------------
------------------------------------------------
solution "Networking"
  configurations { "Debug", "Release" }

  configuration { "Debug" }
    targetdir "bin/debug"

  configuration { "Release" }
    targetdir "bin/release"

  if _ACTION == "clean" then
    os.rmdir("bin/debug", "bin/release")
  end

startproject=NetworkingTest

------------------------------------------------
---------------  Net Test  ------------------
------------------------------------------------
  project "NetworkingTest"
    language "C++"
    kind     "ConsoleApp"

    files  { "include/*.h", "include/engine/**.h", "include/*.hpp", "include/engine/**.hpp",
             "src/*.cpp", "src/NetTest/**.cpp" }
    
    libdirs { "lib\\" }
    if(os.get() == "windows") then
      defines{ "SUBSYSTEM=WINDOWS"}
      debugenvs "PATH=%PATH%;$(ProjectDir)"
    else
      defines{ "SUBSYSTEM=LINUX"}
    end

    includedirs {
    "include",
    "include/**"
    }
    
    if(_ACTION == "gmake") then
      buildoptions {"-std=c++0x"}
      pchheader ( "include/MainPrecompiled.h" )
    else
      pchheader ( "MainPrecompiled.h" )
    end
    pchsource ( "src/MainPrecompiled.cpp" )

    links {"NetworkLib", "wsock32"}

    configuration { "Debug*" }
      defines { "_DEBUG", "DEBUG" }
      flags   { "Symbols" }
      libdirs { "lib/debug" }
      
    configuration { "Release*" }
      defines { "NDEBUG" }
      flags   { "Optimize" }
      libdirs { "lib/release" }

  
---------------------------------------------------
------------------  WEB SERVER  -------------------
---------------------------------------------------

  project "WebServer"
    language "C++"
    kind     "ConsoleApp"

    files  { "include/*.h", "include/engine/**.h", "include/*.hpp", "include/engine/**.hpp",
             "src/*.cpp",
             "include/WebServer/**.h", "include/Webserver/**.hpp",
             "src/WebServer/**.cpp"}
    
    libdirs { "lib\\" }
    if(os.get() == "windows") then
      defines{ "SUBSYSTEM=WINDOWS"}
      debugenvs "PATH=%PATH%;$(ProjectDir)"
    else
      defines{ "SUBSYSTEM=LINUX"}
    end

    includedirs {
    "include",
    "include/**"
    }
    
    if(_ACTION == "gmake") then
      buildoptions {"-std=c++0x"}
      pchheader ( "include/MainPrecompiled.h" )
    else
      pchheader ( "MainPrecompiled.h" )
    end
    pchsource ( "src/MainPrecompiled.cpp" )

    links {"NetworkLib", "wsock32"}

    configuration { "Debug*" }
      defines { "_DEBUG", "DEBUG" }
      flags   { "Symbols" }
      libdirs { "lib/debug" }
      
    configuration { "Release*" }
      defines { "NDEBUG" }
      flags   { "Optimize" }
      libdirs { "lib/release" }

  
  
------------------------------------------------
---------------  FILE Transfer  ------------------
------------------------------------------------
  project "FileTransfer"
    language "C++"
    kind     "ConsoleApp"

    files  { "include/FileShare/*.h", "include/*.h", "include/engine/**.h", "include/FileShare/*.hpp", "include/engine/**.hpp",
             "src/FileShare/*.cpp", "src/*.cpp",
             "include/FileShare/**.h", "include/FileShare/**.hpp",  
             "src/FileShare/**.cpp"}
    
    libdirs { "lib\\" }
    if(os.get() == "windows") then
      defines{ "SUBSYSTEM=WINDOWS"}
      debugenvs "PATH=%PATH%;$(ProjectDir)"
    else
      defines{ "SUBSYSTEM=LINUX"}
    end

    includedirs {
    "include",
    "include/FileShare/",
    "include/FileShare/**",
    "include/engine/",
    "include/engine/**"
    }
    
    if(_ACTION == "gmake") then
      buildoptions {"-std=c++0x"}
      pchheader ( "include/MainPrecompiled.h" )
    else
      pchheader ( "MainPrecompiled.h" )
    end
    pchsource ( "src/MainPrecompiled.cpp" )

    links {"NetworkLib", "wsock32"}

    configuration { "Debug*" }
      defines { "_DEBUG", "DEBUG" }
      flags   { "Symbols" }
      libdirs { "lib/debug" }
      
    configuration { "Release*" }
      defines { "NDEBUG" }
      flags   { "Optimize" }
      libdirs { "lib/release" }

------------------------------------------------
---------------  NetService  ------------------
------------------------------------------------
  project "NetService"
    language "C++"
    kind     "ConsoleApp"

    files  { "include/*.h", "include/engine/**.h", "include/engine/**.hpp", "src/*.cpp", 
             "src/FileShare/io/ThreadedReader.cpp", "include/FileShare/io/ThreadedReader.h",
             "include/FileShare/io/ThreadedSafeQueue.h",
             "src/NetService/*.cpp", "include/NetService/*.h" }
    
    libdirs { "lib\\" }
    if(os.get() == "windows") then
      defines{ "SUBSYSTEM=WINDOWS"}
      debugenvs "PATH=%PATH%;$(ProjectDir)"
    else
      defines{ "SUBSYSTEM=LINUX"}
    end

    includedirs {
    "include",
    "include/NetService/",
    "include/engine/",
    "include/engine/**",
    "include/FileShare/io/"
    }
    
    if(_ACTION == "gmake") then
      buildoptions {"-std=c++0x"}
      pchheader ( "include/MainPrecompiled.h" )
    else
      pchheader ( "MainPrecompiled.h" )
    end
    pchsource ( "src/MainPrecompiled.cpp" )

    links {"NetworkLib", "wsock32"}

    configuration { "Debug*" }
      defines { "_DEBUG", "DEBUG" }
      flags   { "Symbols" }
      libdirs { "lib/debug" }
      
    configuration { "Release*" }
      defines { "NDEBUG" }
      flags   { "Optimize" }
      libdirs { "lib/release" }
      
-----------------------------------------------------
---------------   NETWORK ENGINE    -----------------
-----------------------------------------------------
  project "NetworkLib"
    language "C++"
    kind     "StaticLib"

    files  { "include/engine/**.h", "src/engine/**.cpp", "include/engine/**.hpp"}
    
    libdirs { "lib\\" }
    if(os.get() == "windows") then
      defines{ "SUBSYSTEM=WINDOWS"}
      debugenvs "PATH=%PATH%;$(ProjectDir)"
    else
      defines{ "SUBSYSTEM=LINUX"}
    end

    includedirs {
    "include",
    "include/**"
    }

    if(_ACTION == "gmake") then
      buildoptions {"-std=c++0x"}
      pchheader ( "include/engine/NetworkingPrecompiled.h" )
    else
      pchheader ( "NetworkingPrecompiled.h" )
    end
    pchsource ( "src/engine/NetworkingPrecompiled.cpp" )

    configuration { "Debug*" }
      defines { "_DEBUG", "DEBUG" }
      flags   { "Symbols" }
      libdirs { "lib/debug" }
      
    configuration { "Release*" }
      defines { "NDEBUG" }
      flags   { "Optimize" }
      libdirs { "lib/release" }
	  