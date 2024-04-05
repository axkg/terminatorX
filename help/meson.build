# man page

man_page = configure_file(input: 'terminatorX.1.in',
    output: 'terminatorX.1',
    configuration: doc_config
)

install_man(man_page)

# GNOME help

gnome.yelp('terminatorX-manual',
    sources: 'index.docbook',
    media: 'figures/signalflow.png'
)