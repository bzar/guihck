(define color-button
  (composite rectangle '(width 50 height 50)
    (mouse-area '(on-mouse-down (set-clear-color (get-prop (parent) 'color))
                  width 50
                  height 50))))
      
(create-elements!
  (row '(x 100 y 100)
    (color-button '(color (255 0 0)))
    (color-button '(color (0 255 0)))
    (color-button '(color (0 0 255)))))
                  
    
