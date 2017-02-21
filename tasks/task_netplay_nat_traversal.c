/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2017 - Gregor Richards
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <lists/file_list.h>
#include <string/stdstring.h>

#include "tasks_internal.h"
#include "net/net_natt.h"

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "../network/netplay/netplay.h"
#include "../verbosity.h"

struct nat_traversal_state_data
{
   struct natt_status *nat_traversal_state;
   uint16_t port;
};

static void netplay_nat_traversal_callback(void *task_data,
                               void *user_data, const char *error)
{
   struct nat_traversal_state_data *ntsd =
      (struct nat_traversal_state_data *) task_data;

   free(ntsd);

   netplay_driver_ctl(RARCH_NETPLAY_CTL_FINISHED_NAT_TRAVERSAL, NULL);
}

static void task_netplay_nat_traversal_handler(retro_task_t *task)
{
   struct nat_traversal_state_data *ntsd =
      (struct nat_traversal_state_data *) task->task_data;

   natt_init();

   if (natt_new(ntsd->nat_traversal_state))
      natt_open_port_any(ntsd->nat_traversal_state, ntsd->port, SOCKET_PROTOCOL_TCP);

   task_set_progress(task, 100);
   task_set_finished(task, true);
}

bool task_push_netplay_nat_traversal(struct natt_status *nat_traversal_state, uint16_t port)
{
   struct nat_traversal_state_data *ntsd;
   retro_task_t *task = (retro_task_t*)calloc(1, sizeof(*task));

   if (!task)
      return false;

   ntsd = (struct nat_traversal_state_data *) calloc(1, sizeof(*ntsd));

   if (!ntsd)
   {
      free(task);
      return false;
   }

   ntsd->nat_traversal_state = nat_traversal_state;
   ntsd->port = port;

   task->type     = TASK_TYPE_BLOCKING;
   task->handler  = task_netplay_nat_traversal_handler;
   task->callback = netplay_nat_traversal_callback;
   task->task_data = ntsd;

   task_queue_ctl(TASK_QUEUE_CTL_PUSH, task);

   return true;
}
