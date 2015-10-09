(define (us-ls)
  (us-display-module-info (us-module-info "_header_"))
  (us-display-module-info
   (map (lambda (i) (us-module-info i)) (us-module-ids))))
