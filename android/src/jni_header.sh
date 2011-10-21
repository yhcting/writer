mkdir classes

javac -classpath ${ANDJAR} -d ./classes ./com/yhc/writer/D2d.java
javac -classpath ${ANDJAR} -d ./classes -sourcepath . ./com/yhc/writer/WSheet.java

cd classes
javah com.yhc.writer.D2d
javah com.yhc.writer.WSheet

cd ..
