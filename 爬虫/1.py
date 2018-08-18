#!/usr/bin/python
#coding=utf-8
import urllib2
from lxml import etree
import csv
import sys
reload(sys)
sys.setdefaultencoding('utf-8')


def Open(url):
    Head={
        'User-Agent':'Mozilla/5.0 (Windows; U; Windows NT 5.2) AppleWebKit/525.13 (KHTML, like Gecko) Chrome/0.2.149.27 Safari/525.13'
    }

    reques=urllib2.Request(url=url,headers=Head);
    data=urllib2.urlopen(reques).read()
    return data
def Content(page):
    ht=etree.HTML(page);
    title=ht.xpath("string(//*[@id='JobName'])")
    title = title.replace('\r','').replace('\n','').replace('\t','').replace(' ','')
    company=ht.xpath("string(//*[@id='jobCompany']/a)")
    company=company.replace('\r','').replace('\n','').replace('\t','')
    Type=ht.xpath("string(//*[@id='divMain']/div/div/div[1]/div[1]/ul[2]/li[4])")
    Type=Type.replace('\r','').replace('\n','').replace('\t','')
    Address=ht.xpath("string(//*[@id='currentJobCity'])")
    Address=Address.replace('\r','').replace('\n','').replace('\t','').replace(' ','')
    number=ht.xpath("string(//*[@id='divMain']/div/div/div[1]/div[1]/ul[2]/li[6])")
    number=number.replace('\r','').replace('\n','').replace('\t','')
    time=ht.xpath("string(//*[@id='liJobPublishDate'])")
    time=time.replace('\r','').replace('\n','').replace('\t','').replace(' ','')
    despict=ht.xpath("string(//*[@id='divMain']/div/div/div[1]/div[2]/div[2]/div/div)")[33:-413]
    despict=despict.replace('\r','').replace('\n','').replace('\t','')
    ll=[]
    ll=title,company,Type,Address,number,time,despict
    return ll
def write1(page):
    with open(r'123.csv','ab+') as aaa:
        w=csv.writer(aaa)
        w.writerow(page)
        print "ok"
def showurl(page):
    ht=etree.HTML(page)
    url=ht.xpath('//ul[@class="searchResultListUl"]//div[@class="searchResultJobinfo fr"]//a[@target="_blank"]/@href')
    return url
def ALLurl(data):
    i=1
    Allurl=showurl(data)
    for url in Allurl:
        s="http:"+url
        page=Open(s)
        print i
        i+=1
        list1=Content(page)
        write1(list1)

if __name__=='__main__':
    title=['概述','公司','职位类别','工作地点','招聘人数','最后发布时间','描述']
    with open(r'123.csv','ab+') as aaa:
        w=csv.writer(aaa)
        w.writerow(title)
    for i in range(1,6):
        st="https://xiaoyuan.zhaopin.com/full/industry/0/0_0_0_1_0_0_"+str(i)+"_0"
        st=Open(st)
        ALLurl(st)




