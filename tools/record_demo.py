import subprocess, time, socket, re, signal, struct
from pathlib import Path

ROOT = Path(__file__).parent.parent
SIMULATOR = ROOT / "build/Desktop_Qt_6_11_0-Release/Simulator/drone_sim"
DASHBOARD = ROOT / "build/Desktop_Qt_6_11_0-Release/Dashboard/DashboardApp"
OUTPUT_MP4 = Path("/tmp/garuda_demo.mp4")
PALETTE = Path("/tmp/garuda_palette.png")
OUTPUT_GIF = ROOT / "assets/preview.gif"
WINDOW_TITLE = "Project Garuda"

ARM, DISARM, TAKEOFF, LAND = 1, 2, 3, 4

def send_command(cmd_type, param=0.0, seq=0):
    # "=IBBf": native endian, uint32, uint8, uint8, float — no padding (CommandPacket uses #pragma pack(1))
    packet = struct.pack("=IBBf", seq, cmd_type, 0, param)
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.sendto(packet, ("127.0.0.1", 5000))

def get_window_geometry(title, timeout=15):
    # --sync blocks until window appears (up to timeout seconds)
    result = subprocess.run(
        ["xdotool", "search", "--sync", "--name", title],
        capture_output=True, text=True, timeout=timeout
    )
    win_id = result.stdout.strip().splitlines()[0]

    geo = subprocess.run(
        ["xdotool", "getwindowgeometry", "--shell", win_id],
        capture_output=True, text=True
    )

    # Output looks like: X=100\nY=50\nWIDTH=900\nHEIGHT=600
    values = dict(re.findall(r'(\w+)=(\d+)', geo.stdout))
    return win_id, int(values["X"]), int(values["Y"]), int(values["WIDTH"]), int(values["HEIGHT"])

def get_screen_size():
    # Returns the full display resolution as (width, height).
    result = subprocess.run(["xdotool", "getdisplaygeometry"], capture_output=True, text=True)
    w, h = result.stdout.strip().split()
    return int(w), int(h)

def resize_window(win_id, width, height):
    # Anchor to top-left first so the resized window stays fully on screen.
    subprocess.run(["xdotool", "windowmove", win_id, "0", "0"])
    subprocess.run(["xdotool", "windowsize", win_id, str(width), str(height)])
    time.sleep(0.8)   # let the window manager apply before re-querying geometry

def start_recording(x, y, w, h):
    return subprocess.Popen([
        "ffmpeg", "-y",
        "-f", "x11grab",
        "-r", "10",
        "-video_size", f"{w}x{h}",
        "-i", f":0.0+{x},{y}",
        str(OUTPUT_MP4)],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def stop_recording(proc):
    proc.send_signal(signal.SIGINT) # graceful stop - lets ffmpeg flush the file
    proc.wait()

def convert_to_gif():
    # Pass 1: generate optimal palette from the video.
    # -update 1 tells ffmpeg to write a single PNG rather than an image sequence.
    subprocess.run([
        "ffmpeg", "-y", "-i", str(OUTPUT_MP4),
        "-vf", "fps=10,scale=900:-1:flags=lanczos,palettegen",
        "-update", "1",
        str(PALETTE)], check=True)

    # Pass 2: apply palette. paletteuse takes two inputs (video + palette) so
    # -filter_complex is required — -vf only handles single-input filtergraphs.
    subprocess.run([
        "ffmpeg", "-y",
        "-i", str(OUTPUT_MP4), "-i", str(PALETTE),
        "-filter_complex", "[0:v]fps=10,scale=900:-1:flags=lanczos[x];[x][1:v]paletteuse",
        str(OUTPUT_GIF)], check=True)

def main():
    # cwd=ROOT ensures ./garuda.conf resolves correctly for both processes
    sim  = subprocess.Popen([str(SIMULATOR)], cwd=str(ROOT))
    dash = subprocess.Popen([str(DASHBOARD)], cwd=str(ROOT))

    try:
        print("Waiting for dashboard window...")
        win_id, x, y, w, h = get_window_geometry(WINDOW_TITLE)
        sw, sh = get_screen_size()
        resize_window(win_id, sw, sh)
        win_id, x, y, w, h = get_window_geometry(WINDOW_TITLE)  # re-fetch after resize
        time.sleep(3)   # wait for telemetry UDP threads to be ready before recording

        print("Recording...")
        rec = start_recording(x, y, w, h)
        time.sleep(2)   # record standby state

        send_command(ARM);           time.sleep(1)
        send_command(TAKEOFF, 10.0); time.sleep(12)  # climb + cruise — long enough to show attitude/velocity updating
        send_command(LAND);          time.sleep(10)  # descend + touchdown

        time.sleep(1)
        stop_recording(rec)

        print("Converting to GIF...")
        convert_to_gif()
        print(f"Done -> {OUTPUT_GIF}")

    finally:
        sim.terminate();  dash.terminate()
        sim.wait();       dash.wait()

if __name__ == "__main__":
    main()