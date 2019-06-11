package com.anemomind;

import com.anemomind.subprocess.SubProcessPipelineOptions;
import com.anemomind.subprocess.utils.CallingSubProcessUtilsAnemo;
import com.mongodb.DB;
import com.mongodb.MongoClient;
import com.anemomind.subprocess.configuration.SubProcessConfiguration;
import com.anemomind.subprocess.kernel.SubProcessCommandLineArgs;
import com.anemomind.subprocess.kernel.SubProcessKernel;
import org.apache.beam.sdk.Pipeline;
import org.apache.beam.sdk.io.gcp.pubsub.PubsubIO;
import org.apache.beam.sdk.options.PipelineOptionsFactory;
import org.apache.beam.sdk.transforms.DoFn;
import org.apache.beam.sdk.transforms.ParDo;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;

public class ProcessNewLogs {

    static final Logger LOG = LoggerFactory.getLogger(ProcessNewLogs.class);

    public static void main(String[] args) {

        // declare subprocess option
        SubProcessPipelineOptions options =
                PipelineOptionsFactory.fromArgs(args).withValidation().as(SubProcessPipelineOptions.class);
        Pipeline p = Pipeline.create(options);

        // Setup the Configuration option used with all transforms
        SubProcessConfiguration configuration = options.getSubProcessConfiguration();

        // anemomind's pubsub topic
        String TOPIC = configuration.getTopic();

        // bucket name that contains all boat directories in google storage
        String bucketName = configuration.getBucketName();

       // pipeline with all transformations
        p.apply("ReadBoatIdPubSub", PubsubIO.readStrings().fromTopic(TOPIC))
                .apply("ProcessBoatLogs", ParDo.of(new LogPubSub(configuration, "nautical_processBoatLogs", bucketName)))
                .apply("UploadVMGTable", ParDo.of(new UploadvmgTable(configuration, "nautical_catTargetSpeed")));
        p.run().waitUntilFinish();
    }

    public static class LogPubSub extends DoFn<String, String> {

        String boatid;
        String bucketName;
        private SubProcessConfiguration configuration;
        private String binaryName;
        private String[] directories = new String[2];

        public LogPubSub(SubProcessConfiguration configuration, String binary, String bucketName) {

            this.configuration = configuration;
            this.binaryName = binary;
            this.bucketName = bucketName;
        }

        @Setup
        public void setUp() throws Exception {
            CallingSubProcessUtilsAnemo.setUp(configuration, binaryName);
        }

        @ProcessElement
        public void processElement(@Element String pubsubMessage, OutputReceiver<String> outputReceiver) throws Exception {

            try {

                if (!pubsubMessage.isEmpty()) {

                    // download shared objects.
                    // All SOs are stored in libs directory of soBucket in google storage
                    // this is temporary location, once static binding of all c++ binaries completed,
                    // soGcpToLocal method will be removed
                    String soBucket = configuration.getSoBucket();
                    CallingSubProcessUtilsAnemo.soGcpToLocal(configuration, soBucket);

                    // download boat files
                    directories = CallingSubProcessUtilsAnemo.boatGcpToLocal(configuration, bucketName, pubsubMessage);

                    boatid = pubsubMessage.substring(4);

                    String mongoURI = "mongodb://"+configuration.getMongoHost() +":"+configuration.getMongoPort() +"/"+configuration.getMongoDB();

                    String params = "--dir " + directories[0] +
                            " --dst " + directories[1] +
                            " --boatid " + boatid +
                            " --save-default-calib " +
                            "-t --clean -c " +
                            "--mongo-uri " + mongoURI +
                            " --scale 20";
                    String[] paramsArr = params.split(" ");

                    // Our Library takes a single command in position 0 which it will call nautical_processBoatLogs back in the result
                    SubProcessCommandLineArgs commands = new SubProcessCommandLineArgs();

                    SubProcessCommandLineArgs.Command command = new SubProcessCommandLineArgs.Command(0, paramsArr);
                    commands.putCommand(command);

                    // The ProcessingKernel deals with the execution of the process
                    SubProcessKernel kernel = new SubProcessKernel(configuration, binaryName);

                    // Run the command and work through the results
                    List<String> results = kernel.exec(commands, configuration);
                    for (String result : results) {
                        System.out.println("Process Boat log output ==> " + result);
                    }
                    outputReceiver.output(pubsubMessage);

                } else {
                    LOG.error("message is empty");
                }
            } catch (Exception ex) {
                LOG.error("Error while process boat data ", ex);
                throw ex;
            }
        }
    }

    public static class UploadvmgTable extends DoFn<String, Void> {

        String boatid;
        private SubProcessConfiguration configuration;
        private String binaryName;
        private String query = "";

        public UploadvmgTable(SubProcessConfiguration configuration, String binary) {

            this.configuration = configuration;
            this.binaryName = binary;
        }

        @Setup
        public void setUp() throws Exception {
            CallingSubProcessUtilsAnemo.setUp(configuration, binaryName);
        }

        @ProcessElement
        public void processElement(@Element String msg, OutputReceiver<Void> outputReceiver) throws Exception {

            try {

                if (!msg.isEmpty()) {

                    boatid = msg.substring(4);

                    String params = "--id " + boatid +
                            " " + configuration.getWorkerPath() + msg + "/processed/boat.dat";
                    String[] paramsArr = params.split(" ");

                    System.out.println("Processing boat id: " + boatid);

                    // Our Library takes a single command in position 0 which it will call nautical_processBoatLogs back in the result
                    SubProcessCommandLineArgs commands = new SubProcessCommandLineArgs();

                    SubProcessCommandLineArgs.Command command = new SubProcessCommandLineArgs.Command(0, paramsArr);
                    commands.putCommand(command);

                    // The ProcessingKernel deals with the execution of the process
                    SubProcessKernel kernel = new SubProcessKernel(configuration, binaryName);

                    // Run the command and work through the results
                    List<String> results = kernel.exec(commands, configuration);
                    for (String result : results) {
                        query += result + "";
                    }

                    // processing query return by upload vmg table c++ binary

                    System.out.println("Query return by upload vmg table binary: " + query);
                    MongoClient mongo = new MongoClient(configuration.getMongoHost(), Integer.parseInt(configuration.getMongoPort()));
                    DB db = mongo.getDB(configuration.getMongoDB());
                    db.eval(query);


                } else {
                    LOG.error("message is empty");
                }
            } catch (Exception ex) {
                LOG.error("Error while process boat data ", ex);
                throw ex;
            }
        }
    }
}
