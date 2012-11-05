env = Environment()
env.Replace(PREFIX="#inst")
Export("env")

env.Replace(CXX="clang++", CPPFLAGS=["-std=gnu++11", "-stdlib=libc++", "-O2", "-Wall", "-Werror"], LINKFLAGS=['-std=gnu++11', '-stdlib=libc++'])

SConscript('src/SConscript', variant_dir='#gen/src')
SConscript('tests/SConscript', variant_dir='#gen/tests')

