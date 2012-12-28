env = Environment()
env.Replace(PREFIX="#inst")
Export("env")

env.Replace(CXX="clang++", 
            #CPPFLAGS=["-std=gnu++11", "-stdlib=libc++", "-O2", "-Wall", "-Werror"], 
            CPPFLAGS=["-std=gnu++11", "-stdlib=libc++", "-O0", "-Wall", "-Werror", "-g"], 
            LINKFLAGS=['-std=gnu++11', '-stdlib=libc++'])

SConscript(dirs=['src'], variant_dir='#gen/src')
SConscript('tests/SConscript', variant_dir='#gen/tests')

