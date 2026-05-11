# A VMM for running a Linux guest

The Makefile for this VMM relies on variables from the top-level Makefile. So this Makefile should never be invoked directly.

Configuration edits should mainly be done in `vmm_config.h` and `board/zcu102/linux.dts`.

## ZCU102 Linux Images
To keep the repository smaller, the ZCU102 Linux Images are not stored in the repository. They are stored as a release in this [repo](https://github.com/dornerworks/meta-inspecta-sut). The Makefile will download them automatically if they do not exist on the current filesystem. This does mean that if a newer version is ever needed, the local files need to be deleted:

- `board/zcu102/linux`
- `board/zcu102/rootfs.cpio.gz`
