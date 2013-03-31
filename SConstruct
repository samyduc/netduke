# Scons
# NetDuke by Samy Duc

# const
CPPPATH = ["Includes"]
platform = ARGUMENTS.get('OS', Platform())

env_base = Environment()
env_base.Append(CPPPATH=CPPPATH)

# debug
env_debug = env_base.Clone()
env_debug["CCFLAGS"] = '-g'

# release
env_release = env_base.Clone()
env_release["CCFLAGS"] = '-O2'

# setup
env_to_build = {"build/%s/debug":env_debug, "build/%s/release":env_release}

for build, env in env_to_build.iteritems():
	env.SConscript('lib_static.scons', "env", variant_dir=build % ("static"))
	#env.SConscript('lib_dynamic.scons', "env", variant_dir=build % ("dynamic"))

