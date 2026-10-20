"""Plugins Policy by Piradex — early desktop preview. No VST/AU hosting yet."""
import os, sys, tkinter as tk
from tkinter import ttk, filedialog, messagebox
from pathlib import Path
from math import log10
import wave, array

BG="#091523"; PANEL="#10243a"; FG="#eaf3ff"; BLUE="#268cff"
def plugin_paths():
    if sys.platform=="darwin": return [Path.home()/"Library/Audio/Plug-Ins/VST3",Path("/Library/Audio/Plug-Ins/VST3"),Path("/Library/Audio/Plug-Ins/Components")]
    if sys.platform=="win32": return [Path(os.environ.get("COMMONPROGRAMFILES",r"C:\Program Files\Common Files"))/"VST3",Path(r"C:\Program Files\Steinberg\VstPlugins")]
    return []
def scan():
    found=[]
    for base in plugin_paths():
        if base.exists():
            try:
                found.extend(str(p) for p in base.rglob("*") if p.suffix.lower() in (".vst3",".component",".dll"))
            except OSError: pass
    return sorted(set(found))
class App(tk.Tk):
    def __init__(self):
        super().__init__();self.title("Plugins Policy — by Piradex | Preview");self.geometry("1050x690");self.configure(bg=BG)
        tk.Label(self,text="PLUGINS POLICY",bg=BG,fg=FG,font=("Arial",27,"bold")).pack(anchor="w",padx=26,pady=(20,0))
        tk.Label(self,text="by PIRADEX   •   AUDIO PLUGIN ANALYSIS SUITE",bg=BG,fg=BLUE,font=("Arial",11)).pack(anchor="w",padx=28,pady=(0,18))
        body=tk.Frame(self,bg=BG);body.pack(fill="both",expand=True,padx=24)
        side=tk.Frame(body,bg=PANEL,width=320);side.pack(side="left",fill="y",padx=(0,12));side.pack_propagate(False)
        tk.Label(side,text="INSTALLED PLUGINS",bg=PANEL,fg=FG,font=("Arial",13,"bold")).pack(anchor="w",padx=15,pady=15)
        self.lst=tk.Listbox(side,bg="#0a1a2b",fg=FG,selectbackground=BLUE,borderwidth=0);self.lst.pack(fill="both",expand=True,padx=12,pady=8)
        tk.Button(side,text="Scan plugins",command=self.refresh).pack(fill="x",padx=12,pady=12)
        main=tk.Frame(body,bg=PANEL);main.pack(side="left",fill="both",expand=True)
        tk.Label(main,text="AUTO ANALYZE",bg=PANEL,fg=FG,font=("Arial",20,"bold")).pack(anchor="w",padx=22,pady=(25,8))
        tk.Label(main,text="Preview: plugin discovery only. VST3/AU loading and quality scores are not yet implemented.",bg=PANEL,fg="#9eb4c9",wraplength=530,justify="left").pack(anchor="w",padx=22,pady=10)
        tk.Button(main,text="Select WAV file",command=self.wav_info,bg=BLUE,fg="white",padx=20,pady=10).pack(anchor="w",padx=22,pady=15)
        self.output=tk.Text(main,bg="#0a1a2b",fg=FG,borderwidth=0,height=12,wrap="word");self.output.pack(fill="both",expand=True,padx=22,pady=15)
        self.write("Ready. Select a WAV file to inspect its technical metadata.\n")
        self.refresh()
    def write(self,s):self.output.insert("end",s+"\n")
    def refresh(self):
        self.lst.delete(0,"end");items=scan()
        for p in items:self.lst.insert("end",Path(p).name)
        self.write(f"Discovered {len(items)} plugin packages (not loaded or tested).")
    def wav_info(self):
        p=filedialog.askopenfilename(filetypes=[("WAV audio","*.wav")])
        if not p:return
        try:
            with wave.open(p,"rb") as f:
                self.write(f"File: {Path(p).name}\nChannels: {f.getnchannels()}\nSample rate: {f.getframerate()} Hz\nBit depth: {f.getsampwidth()*8}\nDuration: {f.getnframes()/f.getframerate():.2f} seconds\n")
        except Exception as e:messagebox.showerror("WAV error",str(e))
if __name__=="__main__":App().mainloop()
