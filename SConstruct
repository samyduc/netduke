# Scons
# NetDuke by Samy Duc
import platform
import os

# const
CPPPATH = ["Includes"]
MINGW_TARGET = False

env_base = Environment()
env_base.Append(CPPPATH=CPPPATH)

# custom build by platform
if platform.system() == 'Windows':

	env_base.Append(CPPDEFINES=['WIN32_LEAN_AND_MEAN', 'WINDOWS_TARGET'])

	if MINGW_TARGET:
		# build with gcc on windows (experimental)
		env_base = Environment(tools = ['mingw'], ENV = os.environ)
		env_base.Append(CPPPATH=CPPPATH)
		env_base.Append(CCFLAGS="-Wfatal-errors -DWIN32_LEAN_AND_MEAN")
		env_base.PrependENVPath('PATH', 'C:\\MinGW64\\bin')
		env_base.PrependENVPath('LIB', 'C:\\MinGW64\\lib')

elif platform.system() == 'Linux':
	env_base.Append(CPPDEFINES=['LINUX_TARGET'])
	env_base.Append(CCFLAGS="-std=c++11")
	env_base.Append(CCFLAGS="-Wfatal-errors")
else:
	print("Stopping build : Platform unknown %s" % (platform.system()))
	exit(1)

# debug
env_debug = env_base.Clone()
env_debug.Append(CCFLAGS='-g') 

# release
env_release = env_base.Clone()
env_release.Append(CCFLAGS='-O2')



# setup
env_to_build = {"build/%s/debug":env_debug, "build/%s/release":env_release}

for build, env in env_to_build.iteritems():
	env.SConscript('lib_static.scons', "env", variant_dir=build % ("static"))
	#env.SConscript('lib_dynamic.scons', "env", variant_dir=build % ("dynamic"))

