from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.textinput import TextInput
from kivy.uix.button import Button
from kivy.uix.label import Label
from kivy.uix.scrollview import ScrollView
from kivy.core.window import Window
import ctypes
import os

Window.size = (400, 600)

class NoctApp(App):
    def build(self):
        self.lib = None
        try:
            if os.path.exists('./libnoct.so'):
                self.lib = ctypes.CDLL('./libnoct.so')
                self.lib.run_noct.argtypes = [ctypes.c_char_p]
                self.lib.run_noct.restype = ctypes.c_char_p
            else:
                self.show_error("libnoct.so not found!")
        except Exception as e:
            self.show_error(f"Load error: {str(e)}")

        layout = BoxLayout(orientation='vertical', spacing=10, padding=10)
        
        title = Label(text='[b]Noct Interpreter[/b]', markup=True, size_hint_y=None, height=40)
        layout.add_widget(title)

        self.code_input = TextInput(
            text='func sum(a: Integer, b: Integer): Integer {\n    return a + b;\n}\n\nnew x: Integer = 10;\nnew y: Integer = 20;\nwriteln(sum(x, y));',
            multiline=True, font_size=14, size_hint_y=None, height=300
        )
        layout.add_widget(self.code_input)

        run_btn = Button(text='▶ Run', size_hint_y=None, height=50, background_color=(0.2, 0.6, 0.2, 1))
        run_btn.bind(on_press=self.run_code)
        layout.add_widget(run_btn)

        self.output_label = Label(text='Output:', size_hint_y=None, height=150, halign='left', valign='top', text_size=(380, None))
        scroll = ScrollView(size_hint_y=None, height=150)
        scroll.add_widget(self.output_label)
        layout.add_widget(scroll)

        return layout

    def show_error(self, msg):
        self.output_label.text = f'[color=ff0000]ERROR: {msg}[/color]'

    def run_code(self, instance):
        if self.lib is None:
            self.output_label.text = '[color=ff0000]Library not loaded![/color]'
            return
        code = self.code_input.text.encode('utf-8')
        try:
            result = self.lib.run_noct(code)
            if result:
                output = result.decode('utf-8')
                self.output_label.text = f'[color=00ff00]Output:[/color]\n{output}'
            else:
                self.output_label.text = '[color=ffff00]No output[/color]'
        except Exception as e:
            self.output_label.text = f'[color=ff0000]Error: {str(e)}[/color]'

if __name__ == '__main__':
    NoctApp().run()
