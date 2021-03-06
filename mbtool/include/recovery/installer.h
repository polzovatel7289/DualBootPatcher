/*
 * Copyright (C) 2014-2018  Andrew Gunnerson <andrewgunnerson@gmail.com>
 *
 * This file is part of DualBootPatcher
 *
 * DualBootPatcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DualBootPatcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DualBootPatcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "mbcommon/common.h"
#include "mbcommon/flags.h"
#include "mbdevice/device.h"

#include "util/legacy_property_service.h"
#include "util/roms.h"

namespace mb
{

enum class InstallerFlag : uint8_t
{
    SkipMountingVolumes     = 1 << 0,
};
MB_DECLARE_FLAGS(InstallerFlags, InstallerFlag)
MB_DECLARE_OPERATORS_FOR_FLAGS(InstallerFlags)

class Installer
{
public:
    Installer(std::string zip_file, std::string chroot_dir,
              std::string temp_dir, int interface, int output_fd,
              InstallerFlags flags);
    virtual ~Installer();

    bool start_installation();


protected:
    static const std::string CANCELLED;

    enum class ProceedState
    {
        Continue,
        Fail,
        Cancel
    };

    MB_PRINTF(2, 3)
    void display_msg(const char *fmt, ...);
    virtual void display_msg(std::string_view msg);
    virtual void updater_print(std::string_view msg);
    virtual void command_output(std::string_view line);
    virtual std::string get_install_type() = 0;
    virtual std::unordered_map<std::string, std::string> get_properties();
    virtual ProceedState on_initialize();
    virtual ProceedState on_created_chroot();
    virtual ProceedState on_checked_device();
    virtual ProceedState on_set_up_chroot();
    virtual ProceedState on_mounted_filesystems();
    virtual ProceedState on_pre_install();
    virtual ProceedState on_post_install(bool install_ret);
    virtual ProceedState on_unmounted_filesystems();
    virtual ProceedState on_finished();
    virtual void on_cleanup(ProceedState ret);

    std::string _zip_file;
    std::string _chroot;
    std::string _temp;
    int _interface;
    int _output_fd;
    InstallerFlags _flags;
    bool _passthrough;

    device::Device _device;
    std::string _detected_device;
    std::string _boot_block_dev;
    std::string _recovery_block_dev;
    std::string _system_block_dev;
    std::shared_ptr<Rom> _rom;
    std::string _system_path;
    std::string _cache_path;
    std::string _data_path;

    std::unordered_map<std::string, std::string> _prop;
    std::unordered_map<std::string, std::string> _chroot_prop;
    std::unordered_map<std::string, std::string> _cached_prop;

    LegacyPropertyService _legacy_prop_svc;

    std::string _temp_image_path;
    bool _has_block_image;
    bool _copy_to_temp_image;
    bool _is_aroma;
    bool _use_fuse_exfat;

    std::vector<std::string> _associated_loop_devs;

    std::string in_chroot(const std::string &path) const;

    static bool is_aroma(const std::string &path);


private:
    bool _ran;

    void output_cb(std::string_view line, bool error);
    int run_command(const std::vector<std::string> &argv);
    int run_command_chroot(const std::string &dir,
                           const std::vector<std::string> &argv);

    bool create_chroot();
    bool destroy_chroot() const;
    bool mount_efs() const;

    bool extract_multiboot_files();
    bool set_up_busybox_wrapper();
    bool create_image(const std::string &path, uint64_t size);
    bool system_image_copy(const std::string &source,
                           const std::string &image, bool reverse);
    bool mount_dir_or_image(const std::string &source,
                            const std::string &mount_point,
                            const std::string &loop_target,
                            bool is_image,
                            uint64_t image_size);
    static bool change_root(const std::string &path);
    bool set_up_legacy_properties();
    bool updater_fd_reader(int stdio_fd, int command_fd);
    bool run_real_updater();
    bool run_debug_shell();

    ProceedState install_stage_initialize();
    ProceedState install_stage_create_chroot();
    ProceedState install_stage_set_up_environment();
    ProceedState install_stage_check_device();
    ProceedState install_stage_get_install_type();
    ProceedState install_stage_set_up_chroot();
    ProceedState install_stage_mount_filesystems();
    ProceedState install_stage_installation();
    ProceedState install_stage_unmount_filesystems();
    ProceedState install_stage_finish();
    void install_stage_cleanup(ProceedState ret);
};

int update_binary_main(int argc, char *argv[]);

}
