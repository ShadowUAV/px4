/****************************************************************************
 *
 *   Copyright (c) 2012-2015 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file px4_daemon_app.c
 * daemon application example for PX4 autopilot
 *
 * @author Example User <mail@example.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <px4_config.h>
#include <px4_tasks.h>
#include <px4_posix.h>
// #include <nuttx/sched.h>
#include <sched.h>
#include <poll.h>

#include <systemlib/systemlib.h>
#include <systemlib/err.h>

#include <uORB/uORB.h>
#include <uORB/topics/sensor_combined.h>
#include <uORB/topics/vehicle_gps_position.h>
#include <uORB/topics/actuator_outputs.h>

static bool thread_should_exit = false;		/**< daemon exit flag */
static bool thread_running = false;		/**< daemon status flag */
static bool daemon_poll_error_shown = false;
static int daemon_task;				/**< Handle of daemon task / thread */

/**
 * daemon management function.
 */
__EXPORT int px4_daemon_app_main(int argc, char *argv[]);

/**
 * Mainloop of daemon.
 */
int px4_daemon_thread_main(int argc, char *argv[]);

/**
 * Print the correct usage.
 */
static void usage(const char *reason);

static void
usage(const char *reason)
{
	if (reason) {
		warnx("%s\n", reason);
	}

	warnx("usage: daemon {start|stop|status} [-p <additional params>]\n\n");
}

/**
 * The daemon app only briefly exists to start
 * the background job. The stack size assigned in the
 * Makefile does only apply to this management task.
 *
 * The actual stack size should be set in the call
 * to task_create().
 */
int px4_daemon_app_main(int argc, char *argv[])
{
	if (argc < 2) {
		usage("missing command");
		return 1;
	}

	if (!strcmp(argv[1], "start")) {

		if (thread_running) {
			warnx("daemon already running\n");
			/* this is not an error */
			return 0;
		}

		thread_should_exit = false;
		daemon_task = px4_task_spawn_cmd("daemon",
						 SCHED_DEFAULT,
						 SCHED_PRIORITY_DEFAULT,
						 2000,
						 px4_daemon_thread_main,
						 (argv) ? (char *const *)&argv[2] : (char *const *)NULL);
		return 0;
	}

	if (!strcmp(argv[1], "stop")) {
		thread_should_exit = true;
		return 0;
	}

	if (!strcmp(argv[1], "status")) {
		if (thread_running) {
			warnx("\trunning\n");

		} else {
			warnx("\tnot started\n");
		}

		return 0;
	}

	usage("unrecognized command");
	return 1;
}

int px4_daemon_thread_main(int argc, char *argv[])
{
	/* subscribe to sensor_combined topic */
    // int sensor_sub_fd = orb_subscribe(ORB_ID(sensor_combined));
    int vehicle_gps_position_fd = orb_subscribe(ORB_ID(vehicle_gps_position));
	int actuator_outputs_fd = orb_subscribe(ORB_ID(actuator_outputs));

    // orb_set_interval(sensor_sub_fd, 1000);
    orb_set_interval(vehicle_gps_position_fd, 1000);
    orb_set_interval(actuator_outputs_fd, 1000);

	warnx("[px4_daemon_app] starting\n");

	/* one could wait for multiple topics with this technique, just using one here */
    px4_pollfd_struct_t fds[] =
	{
        // { .fd = sensor_sub_fd,   .events = POLLIN },
        { .fd = vehicle_gps_position_fd,   .events = POLLIN },
        { .fd = actuator_outputs_fd,   .events = POLLIN },

		/* there could be more file descriptors here, in the form like:
		 * { .fd = other_sub_fd,   .events = POLLIN },
		 */
	};

    thread_running = true;

	int error_counter = 0;

	while (!thread_should_exit)
	{
		// warnx("Hello daemon!\n");

        int poll_ret = px4_poll(fds, sizeof(fds) / sizeof(fds[0]), 2000);

		/* handle the poll result */
		if (poll_ret == 0)
		{
			/* this means none of our providers is giving us data */
			PX4_ERR("[px4_daemon_app] Got no data within a second");

		}
		else if (poll_ret < 0)
		{
			/* this is seriously bad - should be an emergency */
            if (((error_counter < 10) || (error_counter % 50 == 0)) && !daemon_poll_error_shown)
			{
                daemon_poll_error_shown = true;
				/* use a counter to prevent flooding (and slowing us down) */
				PX4_ERR("[px4_daemon_app] ERROR return value from poll(): %d", poll_ret);
			}

			error_counter++;

		}
		else
		{
			int display = 0;
			struct vehicle_gps_position_s vehicle_gps_position_raw;
            // struct sensor_combined_s raw;
            struct actuator_outputs_s actuator_outputs_raw;

            /*
            if (fds[0].revents & POLLIN)
			{

                // copy sensors raw data into local buffer
				orb_copy(ORB_ID(sensor_combined), sensor_sub_fd, &raw);
				display = 1;
			}
            */


            if (fds[0].revents & POLLIN)
			{

                // copy position raw data into local buffer
				orb_copy(ORB_ID(vehicle_gps_position), vehicle_gps_position_fd, &vehicle_gps_position_raw);
				display = 1;
            }

            if (fds[1].revents & POLLIN)
			{

                // copy position raw data into local buffer
				orb_copy(ORB_ID(actuator_outputs), actuator_outputs_fd, &actuator_outputs_raw);
				display = 1;
			}

			if (display)
			{
                /* warnx("[px4_daemon_app] Accelerometer:\t%8.4f\t%8.4f\t%8.4f\t\tPosition:\tlat:%8.6f degrees\tlong:%8.6f degrees\talt_ellipsoid:%8.6f m\talt:%8.6f m\tcount: %d\t\r",
						 (double)raw.accelerometer_m_s2[0],
						(double)raw.accelerometer_m_s2[1],
						(double)raw.accelerometer_m_s2[2],
						 (double)vehicle_gps_position_raw.lat/10000000.0,
						 (double)vehicle_gps_position_raw.lon/10000000.0,
						 (double)vehicle_gps_position_raw.alt_ellipsoid/1.0,
                        (double)vehicle_gps_position_raw.alt/1000.0,
                        actuator_outputs_raw.noutputs
						);
                */

                warnx("[px4_daemon_app] Position:\tlat:%8.6f degrees\tlong:%8.6f degrees\talt_ellipsoid:%8.6f m\talt:%8.6f m\tcount: %d\t\r",
                         (double)vehicle_gps_position_raw.lat/10000000.0,
                         (double)vehicle_gps_position_raw.lon/10000000.0,
                         (double)vehicle_gps_position_raw.alt_ellipsoid/1.0,
                        (double)vehicle_gps_position_raw.alt/1000.0,
                        actuator_outputs_raw.noutputs
                        );

                for (int index = 0; index < actuator_outputs_raw.noutputs; index++)
				{
					warnx("ch[%d] = %8.6f\t\t\r",
						  index,
						  (double)actuator_outputs_raw.output[index]
						  );
				}

				sleep(1);
			}

			/* there could be more file descriptors here, in the form like:
			 * if (fds[1..n].revents & POLLIN) {}
			 */
		}
	}

	warnx("[px4_daemon_app] exiting.\n");

	thread_running = false;

	return 0;
}
