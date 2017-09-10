## Design
The library will run a Web Server.  The files served by the Web Server will come from a file system contained
in a partition.  The files are supplied as part of the solution and are a mixture of HTML and JavaScript.  When
a browser connects to the ESP32 hosted Web Server, it is these files that are brought back to the browser and
executed.  The files will then present an attractive user interface for the user to navigate across the distinct
parts of the ESP32 environment.


### File system
The file system used is the FATFS supplied as part of ESP-IDF.

### Browser UI
The browser UI is built from a mixture of HTML5, JavaScript, jQuery and jQuery UI.  Additional open source components
are used for specialized visualization including:

* jstree


## Components

### File system
The file system examination story uses standard IO functions to examine the file system directory structures.  A
JSON representation of the file system is built and sent back to the browser.  The browser then examines the JSON
data received and visualizes it through the "jstree" library,

