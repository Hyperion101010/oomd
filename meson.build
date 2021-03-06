project('oomd', 'cpp',
  version : '0.1.0',
  license : 'GPL2')

cpp_args = ['-std=c++14', '-g']

# '..' is needed as an include directory b/c of how internal FB tooling
# requires a specific header path
inc = include_directories('''
    include
    util
    .
    ..
'''.split())

srcs = files('''
    Log.cpp
    Log.h
    Oomd.cpp
    Oomd.h
    OomdContext.cpp
    OomdContext.h
    PluginRegistry.cpp
    PluginRegistry.h
    config/ConfigCompiler.cpp
    config/ConfigCompiler.h
    config/ConfigTypes.cpp
    config/ConfigTypes.h
    config/JsonConfigParser.cpp
    config/JsonConfigParser.h
    engine/BasePlugin.h
    engine/DetectorGroup.cpp
    engine/DetectorGroup.h
    engine/Engine.cpp
    engine/Engine.h
    engine/Ruleset.cpp
    engine/Ruleset.h
    include/Assert.cpp
    include/Assert.h
    include/Types.h
    plugins/BaseKillPlugin.cpp
    plugins/BaseKillPlugin.h
    plugins/ContinuePlugin.cpp
    plugins/ContinuePlugin.h
    plugins/DumpCgroupOverview.cpp
    plugins/DumpCgroupOverview.h
    util/Fs.cpp
    plugins/MemoryAbove.cpp
    plugins/MemoryAbove.h
    plugins/MemoryReclaim.cpp
    plugins/MemoryReclaim.h
    plugins/PressureAbove.cpp
    plugins/PressureAbove.h
    plugins/PressureRisingBeyond.cpp
    plugins/PressureRisingBeyond.h
    plugins/SwapFree.cpp
    plugins/SwapFree.h
    plugins/KillMemoryGrowth.cpp
    plugins/KillMemoryGrowth.h
    plugins/KillMemoryGrowth-inl.h
    plugins/KillSwapUsage.cpp
    plugins/KillSwapUsage.h
    plugins/KillSwapUsage-inl.h
    plugins/KillPressure.cpp
    plugins/KillPressure.h
    plugins/KillPressure-inl.h
    util/Fs.h
    util/ScopeGuard.h
'''.split())

deps = [dependency('jsoncpp'),
        dependency('threads')]

# Optional dependencies
systemd_dep = dependency('libsystemd', required: false)
if systemd_dep.found()
  srcs += files('''
      plugins/systemd/BaseSystemdPlugin.cpp
      plugins/systemd/BaseSystemdPlugin.h
      plugins/systemd/SystemdRestart-inl.h
      plugins/systemd/SystemdRestart.cpp
      plugins/systemd/SystemdRestart.h
  '''.split())

  deps += [systemd_dep]
endif

oomd_lib = library('oomd',
                   srcs,
                   include_directories : inc,
                   cpp_args : cpp_args,
                   install: true,
                   dependencies : deps)
executable('oomd_bin',
           files('Main.cpp'),
           include_directories : inc,
           cpp_args : cpp_args,
           dependencies : deps,
           install: true,
           install_rpath: join_paths(get_option('prefix'), get_option('libdir')),
           link_with : oomd_lib)

gtest_dep = dependency('gtest', main : true, required : false)
gmock_dep = dependency('gmock', required : false)
if gtest_dep.found() and gmock_dep.found()
    deps += [gtest_dep, gmock_dep]

    # Core tests
    config_tests = executable('oomd_config_tests',
                              files('config/JsonConfigParserTest.cpp'),
                              include_directories : inc,
                              cpp_args : cpp_args,
                              dependencies : deps,
                              link_with : oomd_lib)
    oomd_tests = executable('oomd_oomd_tests',
                            'OomdTest.cpp',
                            include_directories : inc,
                            cpp_args : cpp_args,
                            dependencies : deps,
                            link_with : oomd_lib)
    context_tests = executable('oomd_context_tests',
                            files('OomdContextTest.cpp'),
                            include_directories : inc,
                            cpp_args : cpp_args,
                            dependencies : deps,
                            link_with : oomd_lib)
    util_tests = executable('oomd_util_tests',
                            'util/FsTest.cpp',
                            'util/ScopeGuardTest.cpp',
                            include_directories : inc,
                            cpp_args : cpp_args,
                            dependencies : deps,
                            link_with : oomd_lib)
    log_tests = executable('oomd_log_tests',
                            'LogTest.cpp',
                            include_directories : inc,
                            cpp_args : cpp_args,
                            dependencies : deps,
                            link_with : oomd_lib)
    assert_tests = executable('oomd_assert_tests',
                            'include/AssertTest.cpp',
                            include_directories : inc,
                            cpp_args : cpp_args,
                            dependencies : deps,
                            link_with : oomd_lib)
    compiler_tests = executable('oomd_compiler_tests',
                            'config/ConfigCompilerTest.cpp',
                            include_directories : inc,
                            dependencies : deps,
                            link_with : oomd_lib)
    plugin_tests = executable('oomd_plugin_tests',
                            'plugins/CorePluginsTest.cpp',
                            include_directories : inc,
                            dependencies : deps,
                            link_with : oomd_lib)
    test('context_tests', context_tests)
    test('util_tests', util_tests)
    test('config_tests', config_tests)
    test('oomd_tests', oomd_tests)
    test('log_tests', log_tests)
    test('assert_tests', assert_tests)
    test('compiler_tests', compiler_tests)
    test('plugin_tests', plugin_tests)

    # Optional tests
    if systemd_dep.found()
        systemd_plugin_tests = executable('oomd_systemd_plugin_tests',
                                          'plugins/systemd/SystemdPluginsTest.cpp',
                                          include_directories : inc,
                                          dependencies : deps,
                                          link_with : oomd_lib)
        test('systemd_plugin_tests', systemd_plugin_tests)
    endif

    # Copy over fixtures to build dir so 'ninja test' will work
    r = run_command('rm', '-r', meson.build_root() + '/oomd')
    r = run_command('mkdir', '-p', meson.build_root() + '/oomd')
    if r.returncode() != 0
      error('Failed to create test fixture directory')
    endif

    r = run_command('cp', '-r', meson.source_root() + '/fixtures', meson.build_root() + '/oomd')
    if r.returncode() != 0
      error('Failed to copy test fixtures')
    endif
endif
