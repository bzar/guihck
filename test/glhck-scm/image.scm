(create-elements
  (row '()
    (image '(id img-1 source "img/infantry_1.png"
             width (bind (get-prop! 'source-width))
             height (bind (get-prop! 'source-height))))
    (image '(id img-2 source "img/infantry_1.png" 
             width (bind (* 2 (get-prop! (find-element! 'img-1) 'width))) 
             height (bind (* 2 (get-prop! (find-element! 'img-1) 'height)))))
    (image '(id img-3 source "img/infantry_1.png" 
             width (bind (* 2 (get-prop! (find-element! 'img-2) 'width))) 
             height (bind (* 2 (get-prop! (find-element! 'img-2) 'height)))))))
 