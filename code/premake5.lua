workspace "async-hpp"
   configurations { "Debug", "Release" }
   location "build"

project "async-hpp"
   kind "ConsoleApp"
   language "C++"
   location "build"
      
   targetdir "bin/%{cfg.buildcfg}"

   files { "*.hpp", "*.cpp" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"