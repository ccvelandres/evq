#!/usr/bin/python3
import subprocess
import argparse
import pathlib
import typing
from dataclasses import dataclass
import struct

bin_addr2line = "arm-none-eabi-addr2line"


def decode_hex_string(str, endian="little"):
    value = int.from_bytes(bytes.fromhex(str), byteorder=endian)
    return value


def grab_symbol(exe, addr):
    cmd = "{} -f -e {} 0x{:x}".format(bin_addr2line, exe, addr)
    output = subprocess.check_output(cmd, shell=True)
    return output.decode('utf-8').splitlines()[0]


def get_abs_path(path, alt=None):
    return pathlib.Path(path).absolute() if path is not None else alt


def open_thread_out_file(thread_id, out_path):
    p = pathlib.Path(out_path)
    thread_file_path = "{0}-{2:x}{1}".format(
        pathlib.Path.joinpath(p.parent, p.stem), p.suffix, thread_id)
    return open(thread_file_path, 'w')


# Regex (([0-9a-z]{2}\s){20})
# Expected record format -- unsigned int should be 4 bytes
# Each record should be 5*4 bytes = 20 bytes
# typedef struct __profile_entry
# {
#     unsigned int is_enter;
#     unsigned int thread_id;
#     unsigned int timestamp;
#     void        *this_fn;
#     void        *call_site;
# } __cyg_profile_entry_t;

@dataclass
class Record:
    struct_fmt = "<IIIII"
    size = struct.calcsize(struct_fmt)
    mask_is_enter = 0x1
    mask_is_vector = 0x2

    def __init__(self, str, exe_file) -> None:
        self.struct = struct.unpack(Record.struct_fmt, str)
        self.is_enter = self.struct[0] & Record.mask_is_enter
        self.is_vector = self.struct[0] & Record.mask_is_vector
        self.thread_id = self.struct[1]
        self.timestamp = self.struct[2]
        self.this_fn = self.struct[3]
        self.call_site = self.struct[4]

        self.s_this_fn = grab_symbol(exe=exe_file_path, addr=self.this_fn)
        self.s_call_site = grab_symbol(exe=exe_file_path, addr=self.call_site)
        pass


@dataclass
class ThreadContext:
    out_file: typing.IO
    thread_id: int
    call_depth: int
    timestack: typing.List[int]

    def __init__(self, out_file, thread_id, call_depth=0, timestack=[]) -> None:
        self.out_file = out_file
        self.thread_id = thread_id
        self.call_depth = call_depth
        self.timestack = timestack


parser = argparse.ArgumentParser()
parser.add_argument("-d", "--data-file")
parser.add_argument("-c", "--capture-file")
parser.add_argument("-x", "--exe-file")
parser.add_argument("-o", "--out-file")
parser.add_argument("-t", "--thread-out")
args = parser.parse_args()
print(args)

exe_file_path = get_abs_path(args.exe_file)
capture_file_path = get_abs_path(args.capture_file)
data_file_path = get_abs_path(args.data_file, pathlib.Path(
    capture_file_path).with_suffix(".capture").absolute())
out_file_path = get_abs_path(args.out_file, pathlib.Path(
    capture_file_path).with_suffix(".out").absolute())
thread_out = args.thread_out

print("capture_file_path: {}".format(capture_file_path))
print("data_file_path:    {}".format(data_file_path))
print("out_file_path:     {}".format(out_file_path))
print("exe_file_path:     {}".format(exe_file_path))
print("Record byte size:  {}".format(Record.size))


if args.data_file is None:
    start_signature = [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07]
    end_signature = [0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08]
    # Extract actual capture between start and end signature

    with open(capture_file_path, 'r', encoding='utf-8') as capture_file:
        start = True
        sig_index = 0
        start_pos = 0
        end_pos = 0
        for char in iter(lambda: capture_file.read(3), ''):
            byte = decode_hex_string(char)
            # print(" {:x}:{} {}-{:x}".format(start_signature[sig_index], sig_index,char, byte))

            # Find start and end position for actual capture data
            if start:
                if start_signature[sig_index] == byte:
                    sig_index += 1
                    if sig_index == len(start_signature):
                        start_pos = capture_file.tell() + 1  # +1 for space
                        sig_index = 0
                        start = False
                        print(" start_pos: {}".format(start_pos))
                else:
                    sig_index = 0
            else:
                if end_signature[sig_index] == byte:
                    sig_index += 1
                    if sig_index == len(end_signature):
                        end_pos = capture_file.tell() - (len(end_signature) * 3)
                        sig_index = 0
                        print(" end_pos: {}".format(end_pos))
                        break
                else:
                    sig_index = 0

        # copy needed data to other file
        capture_file.seek(start_pos-1)
        char_per_line = 3 * Record.size
        with open(data_file_path, 'wb') as data_file:
            # copy data and format entry per line
            for char in iter(lambda: capture_file.read(char_per_line), ''):
                char = char.replace(" ", "").strip('\r\n\t ')
                data_file.write(bytes.fromhex(char))
                if capture_file.tell() + char_per_line > end_pos:
                    break

thread_contexts: typing.Dict[int, ThreadContext] = {}
if not thread_out:
    out_file = open(out_file_path, 'w')

with open(data_file_path, 'rb') as data_file:
    for record_bytes in iter(lambda: data_file.read(Record.size), ''):
        record = Record(record_bytes, exe_file_path)

        # open file for thread
        if record.thread_id not in thread_contexts:
            thread_contexts[record.thread_id] = ThreadContext(
                open_thread_out_file(record.thread_id, out_file) if thread_out else out_file, record.thread_id, 0, [])

        # increment call stack if enter
        thread_contexts[record.thread_id].call_depth += 1 if record.is_enter else 0

        decode_string = ""
        decode_string += (" " *
                          (thread_contexts[record.thread_id].call_depth * 2))
        if record.is_enter:
            decode_string += ("{ ")
            decode_string += ("0x{:08x}->0x{:08x}  ".format(
                record.call_site, record.this_fn))
            thread_contexts[record.thread_id].timestack.append(record.timestamp)
            duration = None
        else:
            decode_string += ("} ")
            decode_string += ("0x{:08x}->0x{:08x}  ".format(
                record.this_fn, record.call_site))
            if len(thread_contexts[record.thread_id].timestack) > 0:
                duration = record.timestamp - thread_contexts[record.thread_id].timestack.pop()

        decode_string += ("[{:08x}]  ".format(record.thread_id))
        decode_string += ("[{:08d}]  ".format(record.timestamp))
        if record.is_enter:
            decode_string += ("{}->{} ".format(record.s_call_site,
                              record.s_this_fn))
        else:
            decode_string += ("{}->{} ".format(record.s_this_fn,
                              record.s_call_site))

        if duration is not None:
            decode_string += ("[{:08d}]".format(duration))

        decode_string += ("\n")

        thread_contexts[record.thread_id].out_file.write(decode_string)
        # decrement call stack if exit
        thread_contexts[record.thread_id].call_depth -= 1 if not record.is_enter else 0

if thread_out:
    for [thread_id, context] in thread_contexts.items():
        context.out_file.close()
else:
    out_file.close()