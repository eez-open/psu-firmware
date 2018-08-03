var statusElement = document.getElementById("status");
var progressElement = document.getElementById("progress");
var spinnerElement = document.getElementById("spinner");

var stdinBuffer = [];

var terminal = $("#output").terminal(
  function(command) {
    for (let i = 0; i < command.length; ++i) {
      stdinBuffer.push(command.charCodeAt(i));
    }
    stdinBuffer.push(13);
  },
  {
    greetings: "",
    name: "js_demo",
    prompt: "[[;yellow;]scpi> ]"
  }
);

var Module = {
  preRun: [
    function() {
      ENV.SDL_EMSCRIPTEN_KEYBOARD_ELEMENT = "#canvas";

      var lastCh = null;

      function stdin() {
        var ch = stdinBuffer.shift();
        if (ch !== undefined) {
          lastCh = ch;
        } else {
          // if no input then send 0, null, 0, null, ... to trick emscripten,
          // otherwise it will stop calling this function
          lastCh = lastCh === null ? 0 : null;
        }
        return lastCh;
      }

      FS.init(stdin);
    }
  ],

  postRun: [],

  print: function(text) {
    if (arguments.length > 1) {
      text = Array.prototype.slice.call(arguments).join(" ");
    }

    console.log(text);

    if (text.startsWith("**ERROR")) {
      terminal.error(text);
    } else {
      terminal.echo(text);
    }
  },

  printErr: function(text) {
    if (arguments.length > 1) {
      text = Array.prototype.slice.call(arguments).join(" ");
    }
    console.error(text);
  },

  canvas: (function() {
    var canvas = document.getElementById("canvas");
    // As a default initial behavior, pop up an alert when webgl context is lost. To make your
    // application robust, you may want to override this behavior before shipping!
    // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
    canvas.addEventListener(
      "webglcontextlost",
      function(e) {
        alert("WebGL context lost. You will need to reload the page.");
        e.preventDefault();
      },
      false
    );
    return canvas;
  })(),

  setStatus: function(text) {
    if (!Module.setStatus.last)
      Module.setStatus.last = { time: Date.now(), text: "" };
    if (text === Module.setStatus.last.text) return;
    var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
    var now = Date.now();
    if (m && now - Module.setStatus.last.time < 30) return; // if this is a progress update, skip it if too soon
    Module.setStatus.last.time = now;
    Module.setStatus.last.text = text;
    if (m) {
      text = m[1];
      progressElement.value = parseInt(m[2]) * 100;
      progressElement.max = parseInt(m[4]) * 100;
      progressElement.hidden = false;
      spinnerElement.hidden = false;
    } else {
      progressElement.value = null;
      progressElement.max = null;
      progressElement.hidden = true;
      if (!text) spinnerElement.style.display = "none";
    }
    statusElement.innerHTML = text;
  },

  totalDependencies: 0,

  monitorRunDependencies: function(left) {
    this.totalDependencies = Math.max(this.totalDependencies, left);
    Module.setStatus(
      left
        ? "Preparing... (" +
          (this.totalDependencies - left) +
          "/" +
          this.totalDependencies +
          ")"
        : "All downloads complete."
    );
  }
};

Module.setStatus("Downloading...");

window.onerror = function(event) {
  // TODO: do not warn on ok events like simulating an infinite loop or exitStatus
  Module.setStatus("Exception thrown, see JavaScript console");
  spinnerElement.style.display = "none";
  Module.setStatus = function(text) {
    if (text) Module.printErr("[post-exception status] " + text);
  };
};
