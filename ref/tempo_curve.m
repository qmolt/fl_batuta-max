close all
clear
clc

N = 1;
n = (0 : 0.01 : N); 
c = (-1. : 0.1 : 1.);

cMIN = -0.99;
cMAX = 0.99;
c(c<=cMIN) = cMIN;
c(c>=cMAX) = cMAX;

%-----plot    
figure
hold on
ylabel('y(x)')
xlabel('x')

TitleString = sprintf('curve function → y = pow(x, a(c))');
title(TitleString);

xlim([0 2]);
ylim([0 2]);

%{
c1 = (0.:0.1:0.9);
for i=1:numel(c1)
    y1 = n.^(1./(1.-c1(i)));
    plot(n,y1) 
end

c2 = (-0.9:0.1:0.);
for i=1:numel(c2)
    y2 = n.^(1.+c2(i));
    plot(n,y2) 
end
%}

LegendString = cell(1,numel(c));
for i=1:numel(c)
    sigc = sign(c(i));
    a = (1-c(i)*sigc).^(-sigc);
    y = n.^(a);
    plot(n,y)
    LegendString{i} = sprintf('c = %3.2f → a(c) = %3.2f', c(i), a);
end
legend(LegendString,'Location','best')
hold off
