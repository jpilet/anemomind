exports.config =
  # See docs at http://brunch.readthedocs.org/en/latest/config.html.
  conventions:
    assets:  /^app\/assets\//
    ignored: /^(bower_components\/bootstrap-less(-themes)?|app\/styles\/overrides|(.*?\/)?[_]\w*)/
  modules:
    definition: false
    wrapper: false
  paths:
    watched: ['loader','app','test','vendor']
    public: 'dist'
  sourceMaps:false
  files:

    javascripts:
      joinTo:
        'js/app.js': /^app/
        'js/loader.js': /^loader/
        'js/vendor.js': /^(bower_components|vendor)/
        'test/scenarios.js': /^test(\/|\\)e2e/
      order:
        before: [
          'bower_components/jquery/dist/jquery.js',
          'bower_components/bootstrap/dist/js/bootstrap.js',
          'bower_components/fastclick/lib/fastclick.js',
          'bower_components/underscore/underscore.js'
        ]

    templates:
      joinTo:
        'js/templates.js': /^app/

    stylesheets:
      joinTo:
        'css/vendor.css': /^(bower_components|vendor)/
        'css/app.css': /^app/
      order:
        before: [
          'bower_components/components-font-awesome/css/font-awesome.css',
          'bower_components/bootstrap/dist/css/bootstrap.css',
          'bower_components/bootstrap-sortable/Contents/bootstrap-sortable.css'
        ]
        after: [
        ]


  keyword:
    # file filter
    filePattern: /\.(css|html)$/

    # By default keyword-brunch has these keywords:
    #     {!version!}, {!name!}, {!date!}, {!timestamp!}
    # using information from package.json
    map:
      distRelease: -> (Date.now())


  plugins:
    afterBrunch: [
      'cat bower_components/components-font-awesome/css/font-awesome.css >>dist/css/vendor.css'
      'mkdir dist/fonts/',
      'cp bower_components/components-font-awesome/fonts/* dist/fonts/',
      'cp bower_components/bootstrap/dist/fonts/* dist/fonts/',
    ]  

    angular_templates:
      module: 'app.templates'
      path_transform: (path) -> path.replace('app', '')

    html2js: 
      options:
        base: 'app/assets',
        htmlmin: 
          removeComments: true

    jshint:
      pattern: /^app\/.*\.js$/
      options:
        bitwise: false
        curly: false
      globals:
        jQuery: true
      warnOnly: true

  #  jade:
  #    pretty: yes # Adds pretty-indentation whitespaces to output (false by default)

  # Enable or disable minifying of result js / css files.
  # minify: true
