# desktop file

desktop_file = configure_file(input: 'org.terminatorx.terminatorX.desktop.in',
    output: 'org.terminatorx.terminatorX.desktop',
    configuration: doc_config,
    install: true,
    install_dir: join_paths(get_option('datadir'), 'applications')
)

desktop_validator = find_program('desktop-file-validate', required: false)
if desktop_validator.found()
  test('Validating desktop file', desktop_validator, args: [desktop_file])
endif

# appdata

metainfo = configure_file(input: 'org.terminatorx.terminatorX.metainfo.xml.in',
    output: 'org.terminatorx.terminatorX.metainfo.xml',
    configuration: doc_config,
    install: true,
    install_dir: join_paths(get_option('datadir'), 'metainfo')
)

appdata_validator = find_program('appstreamcli', required: false)
if appdata_validator.found()
  test('Validating desktop file', appdata_validator, 
  args: ['validate', '--no-net', '--explain', metainfo])
endif

# mimetype

mimetype = configure_file(input: 'org.terminatorx.terminatorX-mime.xml.in',
    output: 'org.terminatorx.terminatorX-mime.xml',
    configuration: doc_config,
    install: true,
    install_dir: join_paths(get_option('datadir'), 'mime', 'packages')
)

# handle icons

subdir('icons')