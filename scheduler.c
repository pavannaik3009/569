#include <stdio.h> #include "utlist.h" #include "utils.h"
#include "memory_controller.h" extern long long int CYCLE_VAL;
extern long long unsigned int bank_access_parallelism; long long unsigned int bank_waiting_parallelism;
double alpha_value = 1.2; double slowness[500];
double slowness_max;
int slowness_max_loc;
double slowness_min;
double slowness_max_min_ratio;
long long int t_alone[500];
long long int t_interference[500]; long long int extra_latency = 0; int rank;
int bank;
void init_scheduler_vars()
{
// initialize all scheduler variables here
return;
}
// write queue high water mark; begin draining writes if write queue exceeds this value
#define HI_WM 40
// end write queue drain once write queue has this many writes in it #define LO_WM 20
// 1 means we are in write-drain mode for that channel
int drain_writes[MAX_NUM_CHAN NELS];
/* Each cycle it is possible to issue a valid command from the read or write queues
OR
a valid precharge command to any bank (issue_precharge_command())
OR
a valid precharge_all bank command to a rank (issue_all_bank_precharge_comma nd())
OR
a power_down command (issue_powerdown_command()), programmed either for fast or slow exit mode
OR
a refresh command (issue_refresh_command())
OR
a power_up command (issue_powerup_command())
OR
an activate to a specific row (issue_activate_command()).
If a COL-RD or COL-WR is picked for issue, the scheduler also has the
option to issue an auto-precharge in this cycle (issue_autoprecharge()).
Before issuing a command it is important to check if it is issuable. For the RD/WR queue resident commands, checking the "command_issuable" flag is necessary. To check if the other commands (mentioned above) can be issued, it is important to check one of the following functions: is_precharge_allowed, is_all_bank_precharge_allowed, is_powerdown_fast_allowed, is_powerdown_slow_allowed, is_powerup_allowed, is_refresh_allowed, is_autoprecharge_allowed, is_activate_allowed.
*/
void schedule(int channel, long long int *t_shared)
{
request_t * rd_ptr = NULL; request_t * wr_ptr = NULL;
if ((drain_writes[channel]) && (write_queue_length[channel] != 0))
{
wr_ptr = write_queue_head[channel]; bank_access_parallelism++; }
if ((!drain_writes[channel]) && (read_queue_length[channel] != 0)) {
rd_ptr = read_queue_head[channel]; bank_access_parallelism++;
}
if ((drain_writes[channel]) &&
(write_queue_length[channel] !=
0)) {
if (dram_state[channel][wr_ptr- >dram_addr.rank][wr_ptr- >dram_addr.bank].active_row != wr_ptr->dram_addr.row) // Check if correct
{
extra_latency = 22; // tRCD + tRP
at 800 MHz
t_interference[0] = t_interference[0] + (long long int)(extra_latency/bank_access_par allelism);
} }
if ((!drain_writes[channel]) && (read_queue_length[channel] != 0)) {
if (dram_state[channel][rd_ptr- >dram_addr.rank][rd_ptr- >dram_addr.bank].active_row != rd_ptr->dram_addr.row) // Check if correct
{
extra_latency = 22; // tRCD + tRP at 800 MHz
t_interference[0] = t_interference[0] + (long long int)(extra_latency/bank_access_par allelism);
}
}
int count = 0;
if ((drain_writes[channel]) && (write_queue_length[channel] != 0))
{ LL_FOREACH(write_queue_head[ channel], wr_ptr)
{
if (wr_ptr->command_issuable)
{
count++;
}
}
count--;
bank_waiting_parallelism = count; }
if ((!drain_writes[channel]) && (read_queue_length[channel] != 0))
LL_FOREACH(read_queue_head[ channel], rd_ptr)
{
if (rd_ptr->command_issuable)
{
count++;
}
}
count--;
bank_waiting_parallelism = count; }
int latency;
if ((drain_writes[channel]) && (write_queue_length[channel] != 0))
{
wr_ptr = write_queue_head[channel]; latency = wr_ptr->completion_time - wr_ptr->arrival_time; //needs change
}
if ((!drain_writes[channel]) && (read_queue_length[channel] != 0)) {
	rd_ptr =
read_queue_head[channel];
latency = rd_ptr->completion_time
- rd_ptr->arrival_time; }
count = 0;
if ((drain_writes[channel]) &&
(write_queue_length[channel] !=
0))
{{ LL_FOREACH(write_queue_head[channel], wr_ptr), {
count++; 
if (wr_ptr->command_issuable) {
t_interference[count] =
t_interference[count] +
(latency/(0.5 *
bank_waiting_parallelism)); }} }} 
count = 0;
if ((!drain_writes[channel]) &&
(read_queue_length[channel] != 0))
{
LL_FOREACH(read_queue_head[
channel], rd_ptr)
{
count++;
if (rd_ptr->command_issuable)
{
t_interference[count] =
t_interference[count] +
(latency/(0.5 *
bank_waiting_parallelism)); }{
} slowness_min = slowness[i]; }}
count = 0;
if ((drain_writes[channel]) && (write_queue_length[channel] != 0))
{ LL_FOREACH(write_queue_head[ channel], wr_ptr)
{
count++;
if (wr_ptr->command_issuable)
t_{alone[count] = t_shared[count] - t_interference[count];
slowness[count] = t_shared[count]/t_alone[count];
}
}
}

slowness_max_min_ratio = (slowness_max/slowness_min);
if (slowness_max_min_ratio < alpha_value)
{
// if in write drain mode, keep draining writes until the
// write queue occupancy drops to LO_WM
if (drain_writes[channel] &&
(write_queue_length[channel] >
LO_WM)) {
drain_writes[channel] = 1; // Keep
draining.
}{
else { if(rd_ptr->command_issuable) drain_writes[channel] = 0; // No {
need to drain. issue_request_command(rd_ptr); }
// initiate write drain if either the }
write queue occupancy return;
// has reached the HI_WM , OR, if }
there are no pending read }
// requests else if(write_queue_length[channel] > {
HI_WM) if(drain_writes[channel]) {{
drain_writes[channel] = 1; }
else
{
if (!read_queue_length[channel]) drain_writes[channel] = 1;
}
LL_FOREACH(write_queue_head[ channel], wr_ptr)
{
if ((wr_ptr + slowness_max_loc)- >command_issuable)
{
issue_request_command(wr_ptr + slowness_max_loc);
break;
}
}
return; }
if(!drain_writes[channel])
{ LL_FOREACH(read_queue_head[ channel], rd_ptr)
{
if ((rd_ptr + slowness_max_loc)- >command_issuable)
{
issue_request_command(rd_ptr + slowness_max_loc);
break;
}
// Draining Reads return; // look through the queue and find }
the first request whose }
// command can be issued in this } cycle and issue it
// If in write drain mode, look
through all the write queue
// elements (already arranged in the
order of arrival), and
// issue the command for the first
request that is ready
if(drain_writes[channel])
{
LL_FOREACH(write_queue_head[
channel], wr_ptr)
{
if(wr_ptr->command_issuable)
{
issue_request_command(wr_ptr);
break;
}
}
return
void scheduler_stats() {
/* Nothing to print for now. */ }



