/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.anemomind.subprocess.kernel;

import com.anemomind.subprocess.utils.CallingSubProcessUtilsAnemo;
import com.anemomind.subprocess.utils.FileUtils;
import com.anemomind.subprocess.configuration.SubProcessConfiguration;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

/**
 * This is the process kernel which deals with exec of the subprocess. It also deals with all I/O.
 */
public class SubProcessKernel {

    private static final Logger LOG = LoggerFactory.getLogger(SubProcessKernel.class);

    private static final int MAX_SIZE_COMMAND_LINE_ARGS = 128 * 1024;

    SubProcessConfiguration configuration;
    ProcessBuilder processBuilder;

    private SubProcessKernel() {
    }

    /**
     * Creates the SubProcess Kernel ready for execution. Will deal with all input and outputs to the
     * SubProcess
     *
     * @param options
     * @param binaryName
     */
    public SubProcessKernel(SubProcessConfiguration options, String binaryName) {
        this.configuration = options;
        this.processBuilder = new ProcessBuilder(binaryName);
    }

    private static String createLogEntryFromInputs(List<String> commands) {
        String params;
        if (commands != null) {
            params = String.join(" ", commands);
        } else {
            params = "No-Commands";
        }
        return params;
    }

    // overload exec for anemomind
    public List<String> exec(SubProcessCommandLineArgs commands, SubProcessConfiguration configuration) throws Exception {
        try (CallingSubProcessUtilsAnemo.Permit permit =
                     new CallingSubProcessUtilsAnemo.Permit(processBuilder.command().get(0))) {

            List<String> results = new ArrayList<>();
            String line = null;

                try {
                    Process process = execBinary(processBuilder, commands, configuration);
                    BufferedReader input =
                            new BufferedReader
                                    (new InputStreamReader(process.getInputStream()));
                    while ((line = input.readLine()) != null) {
                        results.add(line);
                    }
                    input.close();
                } catch (Exception ex) {
                    LOG.error("Error running executable ", ex);
                    throw ex;
                }
            return results;
        }
    }

    private ProcessBuilder prepareBuilder(
            ProcessBuilder builder, SubProcessCommandLineArgs commands, SubProcessConfiguration configuration)
            throws IllegalStateException {

        // setting LD_LIBRARY_PATH for anemomind worker node
        builder.environment().put("LD_LIBRARY_PATH", configuration.getWorkerPath());

        // Check we are not over the max size of command line parameters
        if (getTotalCommandBytesAnemo(commands) > MAX_SIZE_COMMAND_LINE_ARGS) {
            throw new IllegalStateException("Command is over 2MB in size");
        }

        appendExecutablePath(builder);


        // adding command line arguments require by nautical_processBoatLogs
        // from index 1, 0 is reserve for the binary itself
        for (SubProcessCommandLineArgs.Command s : commands.getParameters()) {

            String[] cmdArr = s.getValueArr();

            for (int i = 0; i < cmdArr.length; i++) {

                builder.command().add(i + 1, cmdArr[i]);
            }
        }

        return builder;
    }

    private int getTotalCommandBytesAnemo(SubProcessCommandLineArgs commands) {
        int size = 0;
        for (SubProcessCommandLineArgs.Command c : commands.getParameters()) {
            for (int i = 0; i < c.getValueArr().length; i++) {
                size += c.valueArr[i].length();
            }
        }
        return size;
    }


    private Process execBinary(
            ProcessBuilder builder, SubProcessCommandLineArgs commands,SubProcessConfiguration configuration)
            throws Exception {
        try {

            builder = prepareBuilder(builder, commands,configuration);
            Process process = builder.start();

            boolean timeout = !process.waitFor(configuration.getWaitTime(), TimeUnit.SECONDS);

            if (timeout) {
                String log =
                        String.format(
                                "Timeout waiting to run process with parameters %s . "
                                        + "Check to see if your timeout is long enough. Currently set at %s.",
                                createLogEntryFromInputs(builder.command()), configuration.getWaitTime());
                throw new Exception(log);
            }
            return process;

        } catch (Exception ex) {

            LOG.error(
                    String.format(
                            "Error running process with parameters %s error was %s ",
                            createLogEntryFromInputs(builder.command()), ex.getMessage()));
            throw new Exception(ex);
        }
    }


    // Pass the Path of the binary to the SubProcess in Command position 0
    private ProcessBuilder appendExecutablePath(ProcessBuilder builder) {
        String executable = builder.command().get(0);
        if (executable == null) {
            throw new IllegalArgumentException(
                    "No executable provided to the Process Builder... we will do... nothing... ");
        }
        builder
                .command()
                .set(0, FileUtils.getFileResourceId(configuration.getWorkerPath(), executable).toString());
        return builder;
    }
}
