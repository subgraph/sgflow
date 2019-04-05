project(
	'sgflow',
	'c',
	license: 'MIT',
	default_options: [
		'c_std=c11',
		'warning_level=2',
	],
)

add_project_arguments('-DWLR_USE_UNSTABLE', language: 'c')

wayland_client = dependency('wayland-client')
wayland_cursor = dependency('wayland-cursor')
wayland_protos = dependency('wayland-protocols')
wlroots = dependency('wlroots', fallback: ['wlroots', 'wlroots'], version: '>=0.4.1')
gtk = dependency('gtk+-3.0')
xkbcommon = dependency('xkbcommon')

inc = include_directories('include')

src_files = files(
			'src/main.c',
			'src/result.c',
			'src/layer.c'
)

gnome = import('gnome')

resources = gnome.compile_resources('sgflow-resources',
									'ui/sgflow.gresource.xml',
									source_dir : 'ui',
									c_name     : 'sgflow')

# Must define these here as gnome.compile_resources undefines them
add_project_arguments('-Wno-unused-parameter', language: 'c')
add_project_arguments('-Wno-unused-variable', language: 'c')
add_project_arguments('-Wno-deprecated-declarations', language: 'c')

subdir('protocol')

deps = [
	client_protos,
	wayland_client,
	wayland_cursor,
	wayland_protos,
	wlroots,
	gtk,
	xkbcommon
]


executable(
	'sgflow',
	src_files,
	resources,
	include_directories: [inc],
	dependencies: deps,
	install: true
)
