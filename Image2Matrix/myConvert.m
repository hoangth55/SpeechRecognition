function [F] = myConvert(imageFile)
    %
    %
    I = imread(imageFile);
    I = rgb2gray(I);
    
    [M,N] = size(I);
    
%     for x=1:M
%         for y=1:N
%             if (I(x,y) >= 0.2)
%                 I(x,y) = 1;
%             else
%                 I(x,y) = 0;
%             end
%         end
%     end
    
    I = imresize(I, [10 10]);
    
    for x=1:10
        for y=1:10
            if (I(x,y) < 250)
                I(x,y) = 1;
            else I(x,y) = 0;
            end
        end
    end
    
    imshow(I);
  
    F = I;
    