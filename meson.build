project('terminatorX',
    ['c', 'cpp'],
    version : '4.2.1',
    license: 'GPL2+',
    default_options: [
        'buildtype=debugoptimized'
    ]
)

gnome = import('gnome')
cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp') 

src = [
    'src/main.cc',
    'src/tX_audiodevice.cc',
    'src/tX_audiofile.cc',
    'src/tX_capabilities.cc',
    'src/tX_dial.c',
    'src/tX_dialog.cc',
    'src/tX_endian.c',
    'src/tX_engine.cc',
    'src/tX_event.cc',
    'src/tX_extdial.cc',
    'src/tX_flash.c',
    'src/tX_global.c',
    'src/tX_knobloader.c',
    'src/tX_ladspa.cc',
    'src/tX_ladspa_class.cc',
    'src/tX_legacy_global.c',
    'src/tX_legacy_vtt.cc',
    'src/tX_loaddlg.cc',
    'src/tX_maingui.cc',
    'src/tX_midiin.cc',
    'src/tX_mouse.cc',
    'src/tX_panel.cc',
    'src/tX_pbutton.cc',
    'src/tX_prelis.cc',
    'src/tX_seqpar.cc',
    'src/tX_sequencer.cc',
    'src/tX_tape.cc',
    'src/tX_ui_callbacks.cc',
    'src/tX_ui_interface.cc',
    'src/tX_ui_support.cc',
    'src/tX_vtt.cc',
    'src/tX_vttfx.cc',
    'src/tX_vttgui.cc',
    'src/tX_widget.c',
    'src/wav_read.c',
    'src/wav_write.c'
]

includes = include_directories('src')

xxd = find_program('xxd')
copying_src = custom_target('license header', 
    input: ['COPYING'],
    output:  ['copying.h'],
    command: [xxd, '-i', '@INPUT@', '@OUTPUT@'])

tX_knob_src = gnome.compile_resources(
    'tX_knob_resource', 'src/icons/tX_knob.gresource.xml', 
    source_dir: 'src/icons',
    c_name: 'tX_knob'
)

tX_dialog_src = gnome.compile_resources(
    'tX_dialog_resource', 'src/icons/tX_dialog.gresource.xml', 
    source_dir: 'src/icons',
    c_name: 'tX_dialog'
)

gtk_dep = dependency('gtk+-3.0', version : '>= 3.0', required: true)
xml_dep = dependency('libxml-2.0', version : '>= 2.3.9', required: true)
z_dep = dependency('zlib', version: '>= 1.2', required: true)
threads_dep = dependency('threads')
dl_dep = cc.find_library('dl', required : false) 
m_dep = cc.find_library('m', required : false)
mad_dep = dependency('mad', version : '>= 0.15', required: get_option('mad'))
vorbis_dep = dependency('vorbis', version : '>= 1.3', required: get_option('vorbis'))
vorbisfile_dep = dependency('vorbisfile', version : '>= 1.3', required: get_option('vorbis'))
audiofile_dep = dependency('audiofile', version : '>= 0.3', required: get_option('audiofile'))
alsa_dep = dependency('alsa', version : '>= 1.2', required: get_option('alsa'))
jack_dep = dependency('jack', version : '>= 1.9', required: get_option('jack'))
pulse_dep = dependency('libpulse', version : '>= 16', required: get_option('pulse'))
x11_dep = dependency('x11', version : '>= 1.8', required: get_option('x11'))
cap_dep = dependency('libcap', version : '>= 2', required: get_option('capabilities'))
lrdf_dep = dependency('lrdf', version : '>= 0.2.4', required: get_option('lrdf'))
deps = [ gtk_dep, xml_dep, threads_dep, dl_dep, mad_dep, vorbis_dep, vorbisfile_dep, audiofile_dep, alsa_dep, jack_dep, pulse_dep, x11_dep, cap_dep, lrdf_dep ]

ladspa = cc.has_header('ladspa.h', required: true)

if cc.has_argument('-finput-charset=utf-8')
    add_global_arguments('-finput-charset=utf-8', language: 'cpp')
endif

config = configuration_data()
config.set('VERSION', '"'+meson.project_version()+'"')

config.set('PACKAGE', '"'+meson.project_name()+'"')
config.set('USE_STARTUP_NOTIFICATION', true)

if host_machine.endian() == 'big'
    config.set('WORDS_BIGENDIAN', true)
endif

sox = find_program('sox', required: get_option('sox'))
if sox.found()
    config.set('USE_SOX_INPUT', true)
endif

mpg123 = find_program('mpg123', required: get_option('mpg123'))
if mpg123.found() and sox.found()
    config.set('USE_MPG123_INPUT', true)
endif

ogg123 = find_program('ogg123', required: get_option('ogg123'))
if ogg123.found()
    config.set('USE_OGG123_INPUT', true)
endif

if mad_dep.found()
    config.set('USE_MAD_INPUT', true)
endif

if vorbis_dep.found() and vorbisfile_dep.found()
    config.set('USE_VORBIS_INPUT', true)
endif

if audiofile_dep.found()
    config.set('USE_AUDIOFILE_INPUT', true)
endif

if alsa_dep.found()
    config.set('USE_ALSA', true)
    config.set('USE_ALSA_MIDI_IN', true)
endif

if jack_dep.found()
    config.set('USE_JACK', true)
endif

if pulse_dep.found()
    config.set('USE_PULSE', true)
endif

oss_dep = cc.has_header('sys/soundcard.h', required: false)
if oss_dep
    config.set('USE_OSS', true)
endif

if x11_dep.found()
    config.set('USE_X11', true)
endif

if cap_dep.found()
    config.set('USE_CAPABILITIES', true)
endif

if lrdf_dep.found()
    config.set('USE_LRDF', true)
endif

if not get_option('scheduler').disabled()
    config.set('USE_SCHEDULER', true)
endif

if not get_option('tX_debug').disabled()
    config.set('ENABLE_DEBUG_OUTPUT', true)
endif

if get_option('deprecated').enabled()
    add_global_arguments('-DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DG_DISABLE_DEPRECATED', language: 'c')
endif

if not get_option('wav').disabled()
    config.set('USE_BUILTIN_WAV', true)
endif

if get_option('legacy').enabled()
    config.set('ENABLE_TX_LEGACY', true)
endif

tX_mode = [ 'rwxr-xr-x' ]
if not get_option('suidroot').disabled()
    tX_mode = [ 'rwSr-xr-x', 'root' ]
endif

assert(alsa_dep.found() or jack_dep.found() or pulse_dep.found() or oss_dep, 'no supported audio driver found')

configure_file(output : 'config.h',
               configuration : config)

executable(meson.project_name(), 
    src, 
    copying_src,
    tX_knob_src,
    tX_dialog_src,
    include_directories: includes,
    dependencies: deps,
    install: true,
    install_mode: tX_mode
)

# docs and supporting files

doc_config = configuration_data()
doc_config.set('VERSION', meson.project_version())

# man page

man_page = configure_file(input: 'terminatorX.1.in',
    output: 'terminatorX.1',
    configuration: doc_config
)

install_man(man_page)

# RPM spec file

spec_file = configure_file(input: 'terminatorX.spec.in',
    output: 'terminatorX.spec',
    configuration: doc_config
)

# desktop file

desktop_file = configure_file(input: 'terminatorX.desktop',
    output: 'terminatorX.desktop',
    configuration: doc_config,
    install: true,
    install_dir: join_paths(get_option('datadir'), 'applications')
)

desktop_validator = find_program('desktop-file-validate', required: false)
if desktop_validator.found()
  test('Validating desktop file', desktop_validator, args: [desktop_file])
endif

# appdata

appdata = configure_file(input: 'terminatorX.appdata.xml',
    output: 'terminatorX.appdata.xml',
    configuration: doc_config,
    install: true,
    install_dir: join_paths(get_option('datadir'), 'appdata')
)

appdata_validator = find_program('appstreamcli', required: false)
if appdata_validator.found()
  test('Validating desktop file', appdata_validator, 
  args: ['validate', '--no-net', '--explain', appdata])
endif

# GNOME online help

subdir('help')

# icons

subdir('icons')