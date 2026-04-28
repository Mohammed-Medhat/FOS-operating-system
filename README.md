# FOS
An advanced mini kernel development project forked from MIT’s JOS, focusing on core operating system architecture, low-level memory management and robust process synchronization.

## Technical Contributions

- **Memory Management** - Implemented kernel and user-level heap management using first-fit dynamic allocation for shared resources
- **Process and Scheduling** - Developed a preemptive priority-based Round-Robin scheduler integrated with system timers for efficient CPU distribution
- **System Interface** - Built a secure syscall interface with rigorous parameter validation and an interactive command-line shell.
- **IPC & Concurrency** - Engineered synchronization primitives including semaphores and sleeplocks with wait/signal operations, alongside a shared memory management system for safe cross-process communication
- **Optimization** - Integrated fault handling and detection (including clock replacement algorithms as an advanced feature) and optimized allocators for improved system performance ensuring safe memory sharing between processes

##System Architecture

```
kern/
├── mem/
│   ├── kheap.c/.h          # Kernel heap management
│   ├── shared_memory_manager.c  # Shared memory implementation
│   └── chunk_operations.c   # Memory chunk operations
├── trap/
│   ├── fault_handler.c     # Page fault handling
│   ├── trap.c              # Trap handling
│   └── syscall.c/.h        # System call interface
├── sched/
│   └── sched.c             # Process scheduler
├── cpu/
│   └── sched_helpers.c     # Scheduling utilities
├── cmd/
│   ├── commands.c          # Command processing
│   └── command_prompt.c    # Interactive shell
├── conc/
│   ├── channel.c           # Communication channels
│   └── sleeplock.c         # Sleep locks
├── proc/
│   ├── user_environment.c  # Process environment
│   └── working_set_manager.c # Memory working set
└── tests/ # Comprehensive testing
```
### User Space Libraries
```
lib/
├── uheap.c                 # User heap implementation
├── semaphore.c             # Semaphore library
├── syscall.c               # System call wrappers
└── dynamic_allocator.c     # Dynamic memory allocation
```
## Testing

System stability was verified through a comprehensive suite of automated tests, covering:

- **Integration Testing**: Cross-module functionality validation
- **Performance Benchmarking**: Efficiency and speed analysis under high load.
- **Robustness Testing**: Edge-case validation using both pre-validated (SEEN) and hidden (UNSEEN) faculty-provided test scenarios

## Contributors
- [Mohamed Medhat](https://github.com/Mohammed-Medhat)
- [Yasser Reda](https://github.com/Sparkk505)
- [Mariam Khaled](https://github.com/stanmicro9)
- [Salah Eldeen Tarek](https://github.com/inki69)
- [Mariam Ahmed](https://github.com/Mariamahmadd)
- [Youssof Othman](https://github.com/3ossjunior)
