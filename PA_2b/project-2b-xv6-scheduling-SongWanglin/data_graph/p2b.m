M = dlmread('data.csv');
t = M(:,1);ticks1 = M(:,2); ticks2 = M(:,3); ticks3 = M(:,4);
plot(t, ticks1, 'r-', t, ticks2, 'b-', t, ticks3, 'g-');
xlabel("unit of time");  ylabel("ticks");  
title("Ticks for 3 processes with different tickets");
legend("tickets = 3", "tickets = 2", "tickets = 1",'Location','northwest');
saveas (1, "Graph2.png");