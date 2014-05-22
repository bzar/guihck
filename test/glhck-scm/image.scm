(create-elements!
  (column '()
    (row '()
      (image '(id img-1 source "img/infantry_1.png"
              width (bind (observe 'this 'source-width) identity)
              height (bind (observe 'this 'source-height) identity)))
      (image '(id img-2 source "img/infantry_1.png" 
              width (bind (observe 'img-1 'width) (lambda (w) (* w 2)))
              height (bind (observe 'img-1 'height) (lambda (h) (* h 2)))))
      (image '(id img-3 source "img/infantry_1.png" 
              width (bind  (observe 'img-2 'height) (lambda (w) (* w 2)))
              height (bind  (observe 'img-2 'height) (lambda (h) (* h 2))))))
    (row '()
      (image '(id img-1 source "img/infantry_1.png"
              width 36
              height 36))
      (image '(id img-2 source "img/infantry_1.png" 
              width (bind (observe 'img-1 'width) (lambda (w) (* w 2)))
              height (bind (observe 'img-1 'height) (lambda (h) (* h 2)))))
      (image '(id img-3 source "img/infantry_1.png" 
              width (bind  (observe 'img-2 'height) (lambda (w) (* w 2)))
              height (bind  (observe 'img-2 'height) (lambda (h) (* h 2))))))))
  