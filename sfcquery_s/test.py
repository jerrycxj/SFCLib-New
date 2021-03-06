
if __name__=='__main__':
    coast_key_dir = 'D:\\source\\SFCLib\\sfcquery_s\\withkey.txt'
    ccno_dir = 'D:\\source\\SFCLib\\sfcquery_s\\ccno.txt'
    cch_dir = 'D:\\source\\SFCLib\\sfcquery_s\\cch.txt'

    coast_withkey = []
    with open(coast_key_dir, 'r') as f:
        for line in f:
            line = line.split('\n')[0]
            line = line.split(',')
            line = [float(i) for i in line]
            coast_withkey.append(line)
    print ('点数据个数：' + str( len(coast_withkey)) )
    print ('****************************')

    ccno = []
    with open(ccno_dir, 'r') as f:
        for line in f:
            line = line.split('\n')[0]
            line = line.split(',')
            line = [float(i)  for i in line]
            ccno.append(line)
    print ( 'ccno数据个数：'+ str( len(ccno)) )
    print ('****************************')

    cch = []
    with open(cch_dir, 'r') as f:
        for line in f:
            line = line.split('\n')[0]
            line = line.split(',')
            line = [float(i)  for i in line]
            cch.append(line)
    print ( 'cch数据个数：'+ str( len(cch)) )
    print ('****************************')

    def isinTriangle(rect_x1,rect_y1,rect_x2,rect_y2,x,y):
        if( x>=rect_x1 and x<=rect_x2 and y>=rect_y1 and y<=rect_y2):
            return True
        else:
            return False


    ###  查询条件 39150,417436,39170,417496
    count = 0
    for pt in coast_withkey:
        if( isinTriangle(39150,417436,39170,417496,pt[0],pt[1]) ):
           count += 1
    print ( '矩形范围查询个数：'+ str( count ) )
    print ('****************************')

    #### ccno
    count = 0
    range_ccnoLen = 0

    for rg in ccno:
        range_ccnoLen += rg[1]-rg[0]

    for pt in coast_withkey:
        for rg in ccno:
            if pt[2]>=rg[0] and pt[2]<=rg[1]:
                count += 1
                break
    print ( 'ccno查询个数：'+ str( count ) )
    print ('****************************')
    print ( 'ccno range总长度：'+ str( range_ccnoLen ) )
    print ('****************************')


    #### cch
    count = 0
    range_cchLen = 0

    for rg in cch:
        range_cchLen += rg[1]-rg[0]

    for pt in coast_withkey:
        for rg in cch:
            if pt[2]>=rg[0] and pt[2]<=rg[1]:
                count += 1
                break
    print ( 'cch查询个数：'+ str( count ) )
    print ('****************************')
    print ( 'cch range总长度：'+ str( range_cchLen ) )
    print ('****************************')
