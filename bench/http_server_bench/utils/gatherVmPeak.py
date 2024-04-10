import sys, argparse, time, os
"""
Usage: do stuff ... & python pstrace.py $! 2>result
Notes: The script starts with the process to be kept track of, taking its PID ($root).
       It checks /proc/$ID/status for $root and PIDs of all its children, until $root
       exits.
Known issue:
       Fails occassionally, maybe due to it might try to read a process that
       has disappeared. Needs better error handling. Use /usr/bin/time unless
       it does not work.
"""

parser = argparse.ArgumentParser(description="Crude profiler that greps from /proc.")
parser.add_argument('PID', type=str)  # the main PID
args = parser.parse_args()

mainPID = args.PID
print('[log::pstrace] mainPID {0}'.format(mainPID), file=sys.stderr)

relevant_IDs = [mainPID]

usage_this_round_VmPeak = 0
fs = ['/proc/'+f+'/status' for f in os.listdir('/proc')]
for filename in fs:
    ID = filename.split('/')[-2]
    try:
        file = open(filename)
        content = file.readlines()
        file.close()
    except (FileNotFoundError, NotADirectoryError) as e:  # either process returned
        if ID in relevant_IDs: relevant_IDs.remove(ID)
        continue
    line = [_ for _ in content if _.startswith('PPid:')]
    if len(line)!=1: continue
    line = line[0]
    parentID = line.strip().split('\t')[1]  # parent
    # log usage
    if parentID in relevant_IDs or ID in relevant_IDs:
        print('ID {0} PPID: {1}'.format(ID, parentID), file=sys.stderr)
        did_not_return = True
        if ID not in relevant_IDs: relevant_IDs.append(ID)
        for line in content:
            if line.startswith('VmPeak:'):
                use = int(line.replace(' ', '').strip().split('\t')[1].replace('kB', '').strip())
                usage_this_round_VmPeak += use
                break


# VmPeak:	 1018804 kB
print('VmPeak:	 {0} kB'.format(usage_this_round_VmPeak))
