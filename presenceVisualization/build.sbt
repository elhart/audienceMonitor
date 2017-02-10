name := """test"""

version := "1.0-SNAPSHOT"

lazy val root = (project in file(".")).enablePlugins(PlayJava)

scalaVersion := "2.11.7"

resolvers += Resolver.url("Edulify Repository", url("http://edulify.github.io/modules/releases/"))(Resolver.ivyStylePatterns)


libraryDependencies ++= Seq(
  "org.hibernate" % "hibernate-entitymanager" % "5.1.0.Final",
  "mysql" % "mysql-connector-java" % "5.1.36",
  evolutions,
  javaCore,
  javaJdbc,
  javaJpa,
  cache,
  javaWs
)


fork in run := true
PlayKeys.externalizeResources := false