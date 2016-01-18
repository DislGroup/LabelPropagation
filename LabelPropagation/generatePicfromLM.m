% This script uses all the Motion Estimation algorithms written for the
% final project and save their results.

close all
clear all

[filename, pathname] = uigetfile('*.mat', 'ÇëÑ¡Ôñ¶à·ùÍ¼Ïñ£¨ÄæÏòÑ¡Ôñ£©','MultiSelect','on');
if isequal(filename,0) || isequal(pathname,0) % User pressed cancel
    return;
end
framenum=size(filename,2);

for i=1:framenum
    fn{1,i}=[pathname filename{1,i}];
end

for i=1:framenum
    imglabel{i}=load(fn{1,i});
end

xx=load('./data/colortable.mat');
Color_sample = xx.Color_sample ; 

%ClassNum = 13;%kitchen
%ClassNum = 6;%conference
%ClassNum = 3;%book
H = 480;
W = 640;


%color the imagelabel
imageout = uint8(zeros(H,W,3));
R=zeros(H,W);
G=zeros(H,W);
B=zeros(H,W);

for i=1:framenum
     for j=1:H
         for k=1:W
            inde = imglabel{i}.nextFrameLabel(j,k);
            if inde == 0
                R(j,k)=0;
                G(j,k)=0;
                B(j,k)=0;
            else
                R(j,k) = Color_sample(1,inde)/(256*256);
                G(j,k) = mod(Color_sample(1,inde)/256,256);
                B(j,k) = mod(Color_sample(1,inde),256);
            end
         end
     end
     imageout(:,:,1) = (R(:,:));
     imageout(:,:,2) = (G(:,:));
     imageout(:,:,3) = (B(:,:));
     imwrite(uint8(imageout),sprintf('kitchen_%06d.jpg', i+1));
end

fclose('all');