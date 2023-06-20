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
    b.installArtifact(exe);

    const raylib = raylib_build.addRaylib(b, target, optimize);
    exe.linkLibrary(raylib);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run the game");
    run_step.dependOn(&run_cmd.step);

    const onelua_exe = b.addExecutable(.{
        .name = "lua",
        .root_source_file = .{ .path = "lua/onelua.c"},
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    onelua_exe.addIncludePath("lua");
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

    const exercises_exe = b.addExecutable(.{
        .name = "exercises",
        .root_source_file = .{ .path = "learn-lua/exercises.c"},
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    exercises_exe.addIncludePath("lua");
    const exercise_lua_flags = lua_c_flags ++ [_][]const u8{ "-DLUA_USE_APICHECK" };
    exercises_exe.addCSourceFiles(&lua_c_files, &exercise_lua_flags);
    exercises_exe.addIncludePath("raylib/src/external/glfw/deps");
    exercises_exe.addCSourceFile("raylib/src/external/glfw/deps/getopt.c", &.{});

    const lua_step = b.step("lua", "Build the Lua interpreter and exercises");
    lua_step.dependOn(&b.addInstallArtifact(onelua_exe).step);
    lua_step.dependOn(&b.addInstallArtifact(exercises_exe).step);
}
