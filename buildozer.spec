[app]

title = Noct Interpreter
package.name = noctapp
package.domain = com.noctus

source.dir = .
source.include_exts = py,png,jpg,kv,atlas,ttf,txt,so,c,h

requirements = python3,kivy

version = 1.0.0
orientation = portrait
fullscreen = 0
resizable = 0

android.permissions = INTERNET
android.api = 30
android.minapi = 21
android.sdk = 30
android.ndk = 23b
android.strip = True
android.java8 = True
android.use_androidx = True
android.archs = arm64-v8a

[buildozer]
log_level = 2
warn_on_root = 1
