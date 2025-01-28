# WHAT

ScriptWorx is a 9p server meant to facilitate cooperative completion of a process that could be represented by a directed graph (called the flow graph).  It fullfills a similar role to n8n.

The primary goal of ScriptWorx is to facilitate partial-to-full automation of a process or task, with the understanding that not all processes or tasks could be fully automated.

Thus a primary goal of ScriptWorx is to allow for seamless integration of human actors into the workflow represented by the provided graph.

# CONCEPTS

## Task

A single non-intake, non-outtake node in the flow graph.

## Worker

A client connected to ScriptWorx to fulfill a single Task.  They are provided a unique WorkSpace

## WorkSpace

A virtual directory containing an input file/directory, output file/directory, system control files (to communicate status), and possibly an overlay of real files meant to be clickable scripts to enable usability with GUI file managers.

## AdminView

A virtual directory holding the textual representation of the flow graph, control files showing the status of workers, status of intake, status of outtake.

## Producer

A client connected to ScriptWorx to produce work into the flow graph, each piece of work is known as a Job.

## Consumer

A client connected to ScriptWorx to consume the final output of the flow graph.

## Job

A single piece of work to do.  A Job will start at the intake, and will follow the flow graph as directed by the graph's specified directions.  A Job will wait at a Task until a Worker signals that it has completed it.  The Job will then travel to the next stage of the graph as directed.

## Observer

A client connected to observe either the admin view, or a specific WorkSpace in a read-only fasion.  Some control files might be hidden from observers, even if they are read-only themselves.

# WORK FLOW

A single instance of ScriptWorx is required per distinct flow graph.  The flow graph will have an intake, an optional outtake, and at least one Task.  Workers will connect to a Task to recieve a WorkSpace, and use the WorkSpace to complete the task, they will have an input and and output which will be used to provide the work to act on (input) and store the result (output).  A single Worker could represent a human actor, or an instance of a script.  A human actor might be multiple Workers, waiting for the availability of different Tasks.

The WorkSpace file tree will have an empty 'ready' file, a worker who is ready for work will read that file, the file will not EOF until there is a Job in that Task.  ScriptWorx will mark that Job as assigned to the Worker, EOF the ready-file, and wait for completion of the Job by the Worker.

A Job will continue along the graph until it reaches the outtake or final Task.

# WORK SPACE LAYOUT

* /input - file or directory (read-only)
* /output - file or directory (ramfs)
	* mkdir
	* create file
	* rename file
	* change file mode
	* delete file
	* delete directory (check to see if we have children)
	* list a subdirectory
	* read files
	* write files
	* CoW
* /ctl
	* ready - gate (special logic)
	* config
	* history - log of path/times through flow
	* space - get workspace ID
	* done
		* pass
		* fail
		* next
	* options (list of valid keywords to write to `next` file)
* script overlay

























