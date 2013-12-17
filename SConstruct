env = Environment()
env.Replace(PREFIX="#inst")
Export("env")

if env['PLATFORM'] == 'posix':
    cppFlags = ["-std=c++11", "-O2", "-Wall", "-Werror", "-g"]
    linkFlags = []
else:
    cppFlags = ["-std=gnu++11", "-stdlib=libc++", "-O2", "-Wall", "-Werror", "-g"]
    linkFlags = ['-std=gnu++11', '-stdlib=libc++']

env.Replace( CXX="clang++"
           , CPPFLAGS=cppFlags
           , LINKFLAGS=linkFlags
           )

SConscript(dirs=['src'], variant_dir='#gen/src')
SConscript(dirs=['tests'], variant_dir='#gen/tests')

