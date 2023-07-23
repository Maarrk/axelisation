const std = @import("std");
const raylib_build = @import("raylib/src/build.zig");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "axelisation",
        .root_source_file = .{ .path = "src/main.c" },
        .target = target,
        .optimize = optimize,
    });
    exe.addIncludePath("raylib/src");
    addLua(exe);
    exe.addIncludePath("src");
    exe.addCSourceFiles(&[_][]const u8{
        "src/config.c",
    }, &[_][]const u8{});
    b.installArtifact(exe);

    const raylib = raylib_build.addRaylib(b, target, optimize);
    exe.linkLibrary(raylib);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}

fn addLua(exe: *std.Build.LibExeObjStep) void {
    const lua_c_files = [_][]const u8{
        "lua/lapi.c",
        "lua/lauxlib.c",
        "lua/lbaselib.c",
        "lua/lcode.c",
        "lua/lcorolib.c",
        "lua/lctype.c",
        "lua/ldblib.c",
        "lua/ldebug.c",
        "lua/ldo.c",
        "lua/ldump.c",
        "lua/lfunc.c",
        "lua/lgc.c",
        "lua/linit.c",
        "lua/liolib.c",
        "lua/llex.c",
        "lua/lmathlib.c",
        "lua/lmem.c",
        "lua/loadlib.c",
        "lua/lobject.c",
        "lua/lopcodes.c",
        "lua/loslib.c",
        "lua/lparser.c",
        "lua/lstate.c",
        "lua/lstring.c",
        "lua/lstrlib.c",
        "lua/ltable.c",
        "lua/ltablib.c",
        "lua/ltm.c",
        "lua/lundump.c",
        "lua/lutf8lib.c",
        "lua/lvm.c",
        "lua/lzio.c",
    };
    const lua_c_flags = [_][]const u8{
        "-Wall",
        "-O2",
        "-fno-stack-protector",
        "-fno-common",
        "-march=native",
    };
    const debug_flags = lua_c_flags ++ [_][]const u8{ "-DLUA_USE_APICHECK" };

    exe.addIncludePath("lua");
    exe.addCSourceFiles(&lua_c_files, if (exe.optimize == .Debug) &debug_flags else &lua_c_flags);
}