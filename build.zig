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
    const lua_step = b.step("lua", "Build the Lua interpreter");
    lua_step.dependOn(&b.addInstallArtifact(onelua_exe).step);
}
